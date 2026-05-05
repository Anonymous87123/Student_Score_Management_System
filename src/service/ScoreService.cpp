#include "EduSys/service/ScoreService.hpp"

#include <algorithm>
#include <sstream>

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
                return it == courses.end() || it->getTeacherId() != session.getOwnerId();
            }),
            all.end());
        return all;
    }
    // Admin 全量
    return all;
}

std::vector<Score> ScoreService::findByStudent(const Session& session, const std::string& studentId) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=Score.findByStudent)");
    }
    if (session.isStudent() && studentId != session.getOwnerId()) {
        throw PermissionException("Student can only read own scores");
    }
    auto all = scoreRepo_.loadAll();
    all.erase(std::remove_if(all.begin(), all.end(),
        [&](const Score& s) { return s.getStudentId() != studentId; }),
        all.end());

    if (session.isTeacher()) {
        auto courses = courseRepo_.loadAll();
        all.erase(std::remove_if(all.begin(), all.end(),
            [&](const Score& s) {
                auto it = std::find_if(courses.begin(), courses.end(),
                    [&](const Course& c) { return c.getCourseId() == s.getCourseId(); });
                return it == courses.end() || it->getTeacherId() != session.getOwnerId();
            }),
            all.end());
    }
    return all;
}

std::vector<Score> ScoreService::findByCourse(const Session& session, const std::string& courseId) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=Score.findByCourse)");
    }
    if (session.isTeacher()) {
        Course c = requireCourse(courseRepo_, courseId);
        if (c.getTeacherId() != session.getOwnerId()) {
            throw PermissionException("Teacher can only read scores of own courses");
        }
    }
    auto all = scoreRepo_.loadAll();
    all.erase(std::remove_if(all.begin(), all.end(),
        [&](const Score& s) { return s.getCourseId() != courseId; }),
        all.end());

    if (session.isStudent()) {
        all.erase(std::remove_if(all.begin(), all.end(),
            [&](const Score& s) { return s.getStudentId() != session.getOwnerId(); }),
            all.end());
    }
    return all;
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
    if (session.isTeacher() && course.getTeacherId() != session.getOwnerId()) {
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
        if (course.getTeacherId() != session.getOwnerId()) {
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
