#include "EduSys/service/StudentService.hpp"

#include <algorithm>
#include <sstream>

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

void validateStudent(const Student& s) {
    if (s.getId().empty())     throw ValidationException("Student id must not be empty");
    if (s.getName().empty())   throw ValidationException("Student name must not be empty");
    if (s.getEnrollYear() <= 0) throw ValidationException("Student enrollYear must be positive");
}

} // namespace

std::vector<Student> StudentService::listAll(const Session& session) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=Student.listAll)");
    }
    auto all = studentRepo_.loadAll();
    if (session.isStudent()) {
        all.erase(std::remove_if(all.begin(), all.end(),
            [&](const Student& s) { return s.getId() != session.getOwnerId(); }),
            all.end());
    }
    // Admin / Teacher 全量返回
    return all;
}

Student StudentService::findById(const Session& session, const std::string& id) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=Student.findById)");
    }
    if (session.isStudent() && id != session.getOwnerId()) {
        throw PermissionException("Student can only read own record");
    }
    auto all = studentRepo_.loadAll();
    auto it = std::find_if(all.begin(), all.end(),
        [&](const Student& s) { return s.getId() == id; });
    if (it == all.end()) {
        throw ValidationException("Student not found: " + id);
    }
    return *it;
}

void StudentService::create(const Session& session, const Student& student) {
    requireAdmin(session, "Student.create");
    validateStudent(student);

    auto all = studentRepo_.loadAll();
    auto dup = std::find_if(all.begin(), all.end(),
        [&](const Student& s) { return s.getId() == student.getId(); });
    if (dup != all.end()) {
        throw ValidationException("Student id already exists: " + student.getId());
    }
    all.push_back(student);
    studentRepo_.saveAll(all);
    Logger::instance().info("Student created: " + student.getId());
}

void StudentService::update(const Session& session, const Student& student) {
    requireAdmin(session, "Student.update");
    validateStudent(student);

    auto all = studentRepo_.loadAll();
    auto it = std::find_if(all.begin(), all.end(),
        [&](const Student& s) { return s.getId() == student.getId(); });
    if (it == all.end()) {
        throw ValidationException("Student not found: " + student.getId());
    }
    *it = student;
    studentRepo_.saveAll(all);
    Logger::instance().info("Student updated: " + student.getId());
}

void StudentService::remove(const Session& session, const std::string& id) {
    requireAdmin(session, "Student.remove");

    auto students = studentRepo_.loadAll();
    auto it = std::find_if(students.begin(), students.end(),
        [&](const Student& s) { return s.getId() == id; });
    if (it == students.end()) {
        throw ValidationException("Student not found: " + id);
    }

    // 1) 清成绩
    auto scores = scoreRepo_.loadAll();
    const auto scoresBefore = scores.size();
    scores.erase(std::remove_if(scores.begin(), scores.end(),
        [&](const Score& sc) { return sc.getStudentId() == id; }),
        scores.end());
    const auto removedScores = scoresBefore - scores.size();
    scoreRepo_.saveAll(scores);

    // 2) 物理删除该学生关联的登录账号（ownerId == id && role == Student）
    auto users = userRepo_.loadAll();
    const auto usersBefore = users.size();
    users.erase(std::remove_if(users.begin(), users.end(),
        [&](const UserAccount& u) {
            return u.getRole() == RoleType::Student && u.getOwnerId() == id;
        }),
        users.end());
    const auto removedUsers = usersBefore - users.size();
    userRepo_.saveAll(users);

    // 3) 删除学生本人
    students.erase(it);
    studentRepo_.saveAll(students);

    std::ostringstream oss;
    oss << "Student removed (cascade): id=" << id
        << " scores=-" << removedScores
        << " users=-"  << removedUsers;
    Logger::instance().info(oss.str());
}

} // namespace EduSys
