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
    if (c.getTeacherId().empty())  throw ValidationException("Course teacherId must not be empty");
    if (c.getSemester().empty())   throw ValidationException("Course semester must not be empty");
    if (c.getCredit() <= 0.0)      throw ValidationException("Course credit must be positive");
}

} // namespace

std::vector<Course> CourseService::listAll(const Session& session) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=Course.listAll)");
    }
    return courseRepo_.loadAll();
}

Course CourseService::findById(const Session& session, const std::string& courseId) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=Course.findById)");
    }
    auto all = courseRepo_.loadAll();
    auto it = std::find_if(all.begin(), all.end(),
        [&](const Course& c) { return c.getCourseId() == courseId; });
    if (it == all.end()) {
        throw ValidationException("Course not found: " + courseId);
    }
    return *it;
}

void CourseService::create(const Session& session, const Course& course) {
    requireAdmin(session, "Course.create");
    validateCourse(course);

    auto teachers = teacherRepo_.loadAll();
    auto teacherIt = std::find_if(teachers.begin(), teachers.end(),
        [&](const Teacher& t) { return t.getId() == course.getTeacherId(); });
    if (teacherIt == teachers.end()) {
        throw ValidationException("Course teacherId not found: " + course.getTeacherId());
    }

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

    auto teachers = teacherRepo_.loadAll();
    auto teacherIt = std::find_if(teachers.begin(), teachers.end(),
        [&](const Teacher& t) { return t.getId() == course.getTeacherId(); });
    if (teacherIt == teachers.end()) {
        throw ValidationException("Course teacherId not found: " + course.getTeacherId());
    }

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
