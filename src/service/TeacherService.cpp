#include "EduSys/service/TeacherService.hpp"

#include <algorithm>
#include <sstream>
#include <vector>

#include "EduSys/common/Exception.hpp"
#include "EduSys/common/Logger.hpp"
#include "EduSys/common/PasswordHasher.hpp"
#include "EduSys/model/Course.hpp"
#include "EduSys/model/Score.hpp"
#include "EduSys/model/UserAccount.hpp"
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

void validateTeacher(const Teacher& teacher) {
    if (teacher.getId().empty()) {
        throw ValidationException("Teacher id must not be empty");
    }
    if (teacher.getName().empty()) {
        throw ValidationException("Teacher name must not be empty");
    }
}

bool containsCourseId(const std::vector<std::string>& ids, const std::string& id) {
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

} // namespace

std::vector<Teacher> TeacherService::listAll(const Session& session) {
    requireAdmin(session, "Teacher.listAll");
    return teacherRepo_.loadAll();
}

Teacher TeacherService::findById(const Session& session, const std::string& id) {
    requireAdmin(session, "Teacher.findById");

    auto all = teacherRepo_.loadAll();
    auto it = std::find_if(all.begin(), all.end(),
        [&](const Teacher& teacher) { return teacher.getId() == id; });
    if (it == all.end()) {
        throw ValidationException("Teacher not found: " + id);
    }
    return *it;
}

void TeacherService::create(const Session& session,
                            const Teacher& teacher,
                            const std::string& username,
                            const std::string& plainPassword) {
    requireAdmin(session, "Teacher.create");
    validateTeacher(teacher);

    auto teachers = teacherRepo_.loadAll();
    auto dupTeacher = std::find_if(teachers.begin(), teachers.end(),
        [&](const Teacher& t) { return t.getId() == teacher.getId(); });
    if (dupTeacher != teachers.end()) {
        throw ValidationException("Teacher id already exists: " + teacher.getId());
    }

    auto users = userRepo_.loadAll();
    if (!username.empty()) {
        if (plainPassword.empty()) {
            throw ValidationException("Teacher account password must not be empty");
        }
        auto dupUser = std::find_if(users.begin(), users.end(),
            [&](const UserAccount& user) { return user.getUsername() == username; });
        if (dupUser != users.end()) {
            throw ValidationException("Username already exists: " + username);
        }
        users.emplace_back(username, PasswordHasher::hash(plainPassword),
                           RoleType::Teacher, teacher.getId(), true);
    }

    teachers.push_back(teacher);
    teacherRepo_.saveAll(teachers);
    if (!username.empty()) {
        userRepo_.saveAll(users);
    }

    Logger::instance().info("Teacher created: " + teacher.getId());
}

void TeacherService::update(const Session& session, const Teacher& teacher) {
    requireAdmin(session, "Teacher.update");
    validateTeacher(teacher);

    auto teachers = teacherRepo_.loadAll();
    auto it = std::find_if(teachers.begin(), teachers.end(),
        [&](const Teacher& t) { return t.getId() == teacher.getId(); });
    if (it == teachers.end()) {
        throw ValidationException("Teacher not found: " + teacher.getId());
    }

    *it = teacher;
    teacherRepo_.saveAll(teachers);
    Logger::instance().info("Teacher updated: " + teacher.getId());
}

void TeacherService::remove(const Session& session, const std::string& id) {
    requireAdmin(session, "Teacher.remove");

    auto teachers = teacherRepo_.loadAll();
    auto teacherIt = std::find_if(teachers.begin(), teachers.end(),
        [&](const Teacher& teacher) { return teacher.getId() == id; });
    if (teacherIt == teachers.end()) {
        throw ValidationException("Teacher not found: " + id);
    }

    auto courses = courseRepo_.loadAll();
    std::vector<std::string> removedCourseIds;
    std::size_t updatedCourses = 0;
    for (auto& course : courses) {
        if (!course.hasTeacher(id)) {
            continue;
        }

        course.removeTeacherId(id);
        if (course.getTeacherIds().empty()) {
            removedCourseIds.push_back(course.getCourseId());
        } else {
            ++updatedCourses;
        }
    }

    auto scores = scoreRepo_.loadAll();
    const auto scoresBefore = scores.size();
    scores.erase(std::remove_if(scores.begin(), scores.end(),
        [&](const Score& score) {
            return containsCourseId(removedCourseIds, score.getCourseId());
        }),
        scores.end());
    const auto removedScores = scoresBefore - scores.size();
    scoreRepo_.saveAll(scores);

    const auto coursesBefore = courses.size();
    courses.erase(std::remove_if(courses.begin(), courses.end(),
        [&](const Course& course) { return containsCourseId(removedCourseIds, course.getCourseId()); }),
        courses.end());
    const auto removedCourses = coursesBefore - courses.size();
    courseRepo_.saveAll(courses);

    auto users = userRepo_.loadAll();
    const auto usersBefore = users.size();
    users.erase(std::remove_if(users.begin(), users.end(),
        [&](const UserAccount& user) {
            return user.getRole() == RoleType::Teacher && user.getOwnerId() == id;
        }),
        users.end());
    const auto removedUsers = usersBefore - users.size();
    userRepo_.saveAll(users);

    teachers.erase(teacherIt);
    teacherRepo_.saveAll(teachers);

    std::ostringstream oss;
    oss << "Teacher removed (cascade): id=" << id
        << " coursesUpdated=" << updatedCourses
        << " courses=-" << removedCourses
        << " scores=-" << removedScores
        << " users=-" << removedUsers;
    Logger::instance().info(oss.str());
}

} // namespace EduSys
