#include "EduSys/service/CourseService.hpp"

#include <algorithm>
#include <sstream>

#include "EduSys/common/Constants.hpp"
#include "EduSys/common/Exception.hpp"
#include "EduSys/common/Logger.hpp"
#include "EduSys/service/Session.hpp"

namespace EduSys {

namespace {

void requireAdmin(const Session& session, const std::string& op) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=" + op + ")");
    }
    if (!session.isAdmin()) {
        throw PermissionException("Admin role required for op=" + op);
    }
}

void validateCourse(const Course& c) {
    if (c.getCourseId().empty())   throw ValidationException("Course id must not be empty");
    if (c.getCourseName().empty()) throw ValidationException("Course name must not be empty");
    if (c.getTeacherIds().empty()) throw ValidationException("Course teacherId must not be empty");
    if (c.getSemester().empty())   throw ValidationException("Course semester must not be empty");
    if (c.getCredit() <= 0.0)      throw ValidationException("Course credit must be positive");
}

void requireLoggedIn(const Session& session, const std::string& op) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=" + op + ")");
    }
}

void validateCourseTeachers(TeacherRepository& teacherRepo, const Course& course) {
    const auto teacherIds = course.getTeacherIds();
    if (teacherIds.empty()) {
        throw ValidationException("Course teacherId must not be empty");
    }

    auto teachers = teacherRepo.loadAll();
    for (const auto& teacherId : teacherIds) {
        auto teacherIt = std::find_if(teachers.begin(), teachers.end(),
            [&](const Teacher& t) { return t.getId() == teacherId; });
        if (teacherIt == teachers.end()) {
            throw ValidationException("Course teacherId not found: " + teacherId);
        }
    }
}

} // namespace

std::vector<Course> CourseService::listAll(const Session& session) {
    requireLoggedIn(session, "Course.listAll");

    auto all = courseRepo_.loadAll();
    if (session.isTeacher()) {
        all.erase(std::remove_if(all.begin(), all.end(),
            [&](const Course& c) { return !c.hasTeacher(session.getOwnerId()); }),
            all.end());
    }
    return all;
}

std::vector<Course> CourseService::listBySemester(const Session& session, const std::string& semester) {
    requireLoggedIn(session, "Course.listBySemester");
    if (semester.empty()) {
        throw ValidationException("Course semester must not be empty");
    }

    auto all = courseRepo_.loadAll();
    all.erase(std::remove_if(all.begin(), all.end(),
        [&](const Course& c) {
            if (c.getSemester() != semester) {
                return true;
            }
            return session.isTeacher() && !c.hasTeacher(session.getOwnerId());
        }),
        all.end());
    return all;
}

std::vector<Course> CourseService::listByTeacher(const Session& session, const std::string& teacherId) {
    requireLoggedIn(session, "Course.listByTeacher");
    if (teacherId.empty()) {
        throw ValidationException("Teacher id must not be empty");
    }
    if (session.isTeacher() && teacherId != session.getOwnerId()) {
        throw PermissionException("Teacher can only query own courses");
    }

    auto all = courseRepo_.loadAll();
    all.erase(std::remove_if(all.begin(), all.end(),
        [&](const Course& c) { return !c.hasTeacher(teacherId); }),
        all.end());
    return all;
}

std::vector<Course> CourseService::listByTeacherAndSemester(const Session& session,
                                                            const std::string& teacherId,
                                                            const std::string& semester) {
    requireLoggedIn(session, "Course.listByTeacherAndSemester");
    if (teacherId.empty()) {
        throw ValidationException("Teacher id must not be empty");
    }
    if (semester.empty()) {
        throw ValidationException("Course semester must not be empty");
    }
    if (session.isTeacher() && teacherId != session.getOwnerId()) {
        throw PermissionException("Teacher can only query own courses");
    }

    auto all = courseRepo_.loadAll();
    all.erase(std::remove_if(all.begin(), all.end(),
        [&](const Course& c) {
            return c.getSemester() != semester || !c.hasTeacher(teacherId);
        }),
        all.end());
    return all;
}

Course CourseService::findById(const Session& session, const std::string& courseId) {
    requireLoggedIn(session, "Course.findById");

    auto all = courseRepo_.loadAll();
    auto it = std::find_if(all.begin(), all.end(),
        [&](const Course& c) { return c.getCourseId() == courseId; });
    if (it == all.end()) {
        throw ValidationException("Course not found: " + courseId);
    }
    if (session.isTeacher() && !it->hasTeacher(session.getOwnerId())) {
        throw PermissionException("Teacher can only read own courses");
    }
    return *it;
}

void CourseService::create(const Session& session, const Course& course) {
    requireAdmin(session, "Course.create");
    validateCourse(course);

    validateCourseTeachers(teacherRepo_, course);

    auto courses = courseRepo_.loadAll();
    auto dup = std::find_if(courses.begin(), courses.end(),
        [&](const Course& c) { return c.getCourseId() == course.getCourseId(); });
    if (dup != courses.end()) {
        throw ValidationException("Course id already exists: " + course.getCourseId());
    }
    courses.push_back(course);
    courseRepo_.saveAll(courses);
    Logger::instance().info("Course created: " + course.getCourseId());
}

void CourseService::update(const Session& session, const Course& course) {
    requireAdmin(session, "Course.update");
    validateCourse(course);

    validateCourseTeachers(teacherRepo_, course);

    auto courses = courseRepo_.loadAll();
    auto it = std::find_if(courses.begin(), courses.end(),
        [&](const Course& c) { return c.getCourseId() == course.getCourseId(); });
    if (it == courses.end()) {
        throw ValidationException("Course not found: " + course.getCourseId());
    }
    *it = course;
    courseRepo_.saveAll(courses);
    Logger::instance().info("Course updated: " + course.getCourseId());
}

void CourseService::remove(const Session& session, const std::string& courseId) {
    requireAdmin(session, "Course.remove");

    auto courses = courseRepo_.loadAll();
    auto it = std::find_if(courses.begin(), courses.end(),
        [&](const Course& c) { return c.getCourseId() == courseId; });
    if (it == courses.end()) {
        throw ValidationException("Course not found: " + courseId);
    }

    // 1) 清成绩
    auto scores = scoreRepo_.loadAll();
    const auto scoresBefore = scores.size();
    scores.erase(std::remove_if(scores.begin(), scores.end(),
        [&](const Score& sc) { return sc.getCourseId() == courseId; }),
        scores.end());
    const auto removedScores = scoresBefore - scores.size();
    scoreRepo_.saveAll(scores);

    // 2) 删课程本身
    courses.erase(it);
    courseRepo_.saveAll(courses);

    std::ostringstream oss;
    oss << "Course removed (cascade): id=" << courseId
        << " scores=-" << removedScores;
    Logger::instance().info(oss.str());
}

} // namespace EduSys
