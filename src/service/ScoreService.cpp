#include "EduSys/service/ScoreService.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_set>

#include "EduSys/common/Constants.hpp"
#include "EduSys/common/Exception.hpp"
#include "EduSys/common/Logger.hpp"
#include "EduSys/service/Session.hpp"

namespace EduSys {

namespace {

bool sameKey(const Score& a, const std::string& sid, const std::string& cid, const std::string& sem) {
    return a.getStudentId() == sid && a.getCourseId() == cid && a.getSemester() == sem;
}

void validateRange(double v, const char* field) {
    if (v < SCORE_MIN || v > SCORE_MAX) {
        std::ostringstream oss;
        oss << field << " out of range [" << SCORE_MIN << "," << SCORE_MAX << "]: " << v;
        throw ValidationException(oss.str());
    }
}

// 找到 score 关联课程；不存在则抛 ValidationException。
Course requireCourse(CourseRepository& cr, const std::string& courseId) {
    auto courses = cr.loadAll();
    auto it = std::find_if(courses.begin(), courses.end(),
        [&](const Course& c) { return c.getCourseId() == courseId; });
    if (it == courses.end()) {
        throw ValidationException("Course not found: " + courseId);
    }
    return *it;
}

void requireStudentExists(StudentRepository& sr, const std::string& studentId) {
    auto students = sr.loadAll();
    auto it = std::find_if(students.begin(), students.end(),
        [&](const Student& s) { return s.getId() == studentId; });
    if (it == students.end()) {
        throw ValidationException("Student not found: " + studentId);
    }
}

} // namespace

std::vector<Score> ScoreService::listAll(const Session& session) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=Score.listAll)");
    }
    auto all = scoreRepo_.loadAll();

    if (session.isStudent()) {
        all.erase(std::remove_if(all.begin(), all.end(),
            [&](const Score& s) { return s.getStudentId() != session.getOwnerId(); }),
            all.end());
        return all;
    }
    if (session.isTeacher()) {
        // 教师只看自己授课的课程的成绩
        auto courses = courseRepo_.loadAll();
        all.erase(std::remove_if(all.begin(), all.end(),
            [&](const Score& s) {
                auto it = std::find_if(courses.begin(), courses.end(),
                    [&](const Course& c) { return c.getCourseId() == s.getCourseId(); });
                return it == courses.end() || !it->hasTeacher(session.getOwnerId());
            }),
            all.end());
        return all;
    }
    // Admin 全量
    return all;
}

std::vector<Score> ScoreService::query(const Session& session,
                                       const std::string& studentId,
                                       const std::string& courseId,
                                       const std::string& className,
                                       const std::string& semester) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=Score.query)");
    }
    if (session.isStudent() && !studentId.empty() && studentId != session.getOwnerId()) {
        throw PermissionException("Student can only read own scores");
    }
    if (session.isTeacher() && !courseId.empty()) {
        Course c = requireCourse(courseRepo_, courseId);
        if (!c.hasTeacher(session.getOwnerId())) {
            throw PermissionException("Teacher can only read scores of own courses");
        }
    }
    if (session.isTeacher() && courseId.empty()) {
        auto teacherCourses = courseRepo_.loadAll();
        teacherCourses.erase(std::remove_if(teacherCourses.begin(), teacherCourses.end(),
            [&](const Course& c) { return !c.hasTeacher(session.getOwnerId()); }),
            teacherCourses.end());
        if (!className.empty()) {
            const auto classStudents = studentRepo_.loadAll();
            std::unordered_set<std::string> allowedStudentIds;
            for (const auto& student : classStudents) {
                if (student.getClassName() == className) {
                    allowedStudentIds.insert(student.getId());
                }
            }
            auto filtered = listAll(session);
            filtered.erase(std::remove_if(filtered.begin(), filtered.end(),
                [&](const Score& score) {
                    if (allowedStudentIds.find(score.getStudentId()) == allowedStudentIds.end()) {
                        return true;
                    }
                    const auto it = std::find_if(teacherCourses.begin(), teacherCourses.end(),
                        [&](const Course& c) { return c.getCourseId() == score.getCourseId(); });
                    return it == teacherCourses.end();
                }),
                filtered.end());
            if (!semester.empty()) {
                filtered.erase(std::remove_if(filtered.begin(), filtered.end(),
                    [&](const Score& score) { return score.getSemester() != semester; }),
                    filtered.end());
            }
            if (!studentId.empty()) {
                filtered.erase(std::remove_if(filtered.begin(), filtered.end(),
                    [&](const Score& score) { return score.getStudentId() != studentId; }),
                    filtered.end());
            }
            return filtered;
        }
    }

    std::unordered_set<std::string> classStudentIds;
    if (!className.empty()) {
        const auto students = studentRepo_.loadAll();
        for (const auto& student : students) {
            if (student.getClassName() == className) {
                classStudentIds.insert(student.getId());
            }
        }
    }

    auto scores = listAll(session);
    scores.erase(std::remove_if(scores.begin(), scores.end(),
        [&](const Score& score) {
            if (!studentId.empty() && score.getStudentId() != studentId) {
                return true;
            }
            if (!courseId.empty() && score.getCourseId() != courseId) {
                return true;
            }
            if (!semester.empty() && score.getSemester() != semester) {
                return true;
            }
            if (!className.empty() && classStudentIds.find(score.getStudentId()) == classStudentIds.end()) {
                return true;
            }
            return false;
        }),
        scores.end());
    return scores;
}

std::vector<Score> ScoreService::findByStudent(const Session& session, const std::string& studentId) {
    if (studentId.empty()) {
        throw ValidationException("Student id must not be empty");
    }
    return query(session, studentId, {}, {}, {});
}

std::vector<Score> ScoreService::findByCourse(const Session& session, const std::string& courseId) {
    if (courseId.empty()) {
        throw ValidationException("Course id must not be empty");
    }
    return query(session, {}, courseId, {}, {});
}

std::vector<Score> ScoreService::findByClass(const Session& session, const std::string& className) {
    if (className.empty()) {
        throw ValidationException("Class name must not be empty");
    }
    return query(session, {}, {}, className, {});
}

std::vector<Score> ScoreService::findBySemester(const Session& session, const std::string& semester) {
    if (semester.empty()) {
        throw ValidationException("Semester must not be empty");
    }
    return query(session, {}, {}, {}, semester);
}

std::vector<Score> ScoreService::findByStudentAndCourse(const Session& session,
                                                        const std::string& studentId,
                                                        const std::string& courseId) {
    if (studentId.empty()) {
        throw ValidationException("Student id must not be empty");
    }
    if (courseId.empty()) {
        throw ValidationException("Course id must not be empty");
    }
    return query(session, studentId, courseId, {}, {});
}

std::vector<Score> ScoreService::findByStudentAndSemester(const Session& session,
                                                          const std::string& studentId,
                                                          const std::string& semester) {
    if (studentId.empty()) {
        throw ValidationException("Student id must not be empty");
    }
    if (semester.empty()) {
        throw ValidationException("Semester must not be empty");
    }
    return query(session, studentId, {}, {}, semester);
}

std::vector<Score> ScoreService::findByStudentCourseAndSemester(const Session& session,
                                                                const std::string& studentId,
                                                                const std::string& courseId,
                                                                const std::string& semester) {
    if (studentId.empty()) {
        throw ValidationException("Student id must not be empty");
    }
    if (courseId.empty()) {
        throw ValidationException("Course id must not be empty");
    }
    if (semester.empty()) {
        throw ValidationException("Semester must not be empty");
    }
    return query(session, studentId, courseId, {}, semester);
}

std::vector<Score> ScoreService::findByCourseAndSemester(const Session& session,
                                                         const std::string& courseId,
                                                         const std::string& semester) {
    if (courseId.empty()) {
        throw ValidationException("Course id must not be empty");
    }
    if (semester.empty()) {
        throw ValidationException("Semester must not be empty");
    }
    return query(session, {}, courseId, {}, semester);
}

std::vector<Score> ScoreService::findByClassAndSemester(const Session& session,
                                                        const std::string& className,
                                                        const std::string& semester) {
    if (className.empty()) {
        throw ValidationException("Class name must not be empty");
    }
    if (semester.empty()) {
        throw ValidationException("Semester must not be empty");
    }
    return query(session, {}, {}, className, semester);
}

std::vector<Score> ScoreService::findByCourseAndClass(const Session& session,
                                                      const std::string& courseId,
                                                      const std::string& className) {
    if (courseId.empty()) {
        throw ValidationException("Course id must not be empty");
    }
    if (className.empty()) {
        throw ValidationException("Class name must not be empty");
    }
    return query(session, {}, courseId, className, {});
}

std::vector<Score> ScoreService::findByCourseClassAndSemester(const Session& session,
                                                              const std::string& courseId,
                                                              const std::string& className,
                                                              const std::string& semester) {
    if (courseId.empty()) {
        throw ValidationException("Course id must not be empty");
    }
    if (className.empty()) {
        throw ValidationException("Class name must not be empty");
    }
    if (semester.empty()) {
        throw ValidationException("Semester must not be empty");
    }
    return query(session, {}, courseId, className, semester);
}

void ScoreService::upsert(const Session& session, const Score& score) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=Score.upsert)");
    }
    if (session.isStudent()) {
        throw PermissionException("Student is not allowed to write scores");
    }

    // 字段层校验
    if (score.getStudentId().empty() || score.getCourseId().empty() || score.getSemester().empty()) {
        throw ValidationException("Score key fields (studentId/courseId/semester) must not be empty");
    }
    validateRange(score.getUsualScore(), "usualScore");
    validateRange(score.getFinalScore(), "finalScore");
    validateRange(score.getTotalScore(), "totalScore");

    // 关联完整性校验
    requireStudentExists(studentRepo_, score.getStudentId());
    Course course = requireCourse(courseRepo_, score.getCourseId());

    // 教师只能改自己授课的课程
    if (session.isTeacher() && !course.hasTeacher(session.getOwnerId())) {
        throw PermissionException("Teacher can only write scores of own courses");
    }

    auto scores = scoreRepo_.loadAll();
    auto it = std::find_if(scores.begin(), scores.end(),
        [&](const Score& s) {
            return sameKey(s, score.getStudentId(), score.getCourseId(), score.getSemester());
        });
    const bool isUpdate = (it != scores.end());
    if (isUpdate) {
        *it = score;
    } else {
        scores.push_back(score);
    }
    scoreRepo_.saveAll(scores);

    std::ostringstream oss;
    oss << "Score " << (isUpdate ? "updated" : "created")
        << ": " << score.getStudentId() << "/" << score.getCourseId()
        << "/" << score.getSemester();
    Logger::instance().info(oss.str());
}

void ScoreService::remove(const Session& session,
                          const std::string& studentId,
                          const std::string& courseId,
                          const std::string& semester) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=Score.remove)");
    }
    if (session.isStudent()) {
        throw PermissionException("Student is not allowed to delete scores");
    }
    if (session.isTeacher()) {
        Course course = requireCourse(courseRepo_, courseId);
        if (!course.hasTeacher(session.getOwnerId())) {
            throw PermissionException("Teacher can only delete scores of own courses");
        }
    }

    auto scores = scoreRepo_.loadAll();
    auto it = std::find_if(scores.begin(), scores.end(),
        [&](const Score& s) { return sameKey(s, studentId, courseId, semester); });
    if (it == scores.end()) {
        throw ValidationException(
            "Score not found: " + studentId + "/" + courseId + "/" + semester);
    }
    scores.erase(it);
    scoreRepo_.saveAll(scores);

    Logger::instance().info(
        "Score removed: " + studentId + "/" + courseId + "/" + semester);
}

} // namespace EduSys
