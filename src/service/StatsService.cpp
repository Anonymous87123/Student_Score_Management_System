#include "EduSys/service/StatsService.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_map>

#include "EduSys/common/Constants.hpp"
#include "EduSys/common/Exception.hpp"
#include "EduSys/service/Session.hpp"

namespace EduSys {

namespace {

// 4.0 制分段：≥90→4.0、85-89→3.7、80-84→3.3、75-79→3.0、
//             70-74→2.7、65-69→2.3、60-64→2.0、<60→0.0。
double gpaPointFor(double total) {
    if (total >= 90.0) return 4.0;
    if (total >= 85.0) return 3.7;
    if (total >= 80.0) return 3.3;
    if (total >= 75.0) return 3.0;
    if (total >= 70.0) return 2.7;
    if (total >= 65.0) return 2.3;
    if (total >= 60.0) return 2.0;
    return 0.0;
}

void requireLoggedIn(const Session& s, const std::string& op) {
    if (!s.isLoggedIn()) {
        throw AuthException("Not logged in (op=" + op + ")");
    }
}

// 在 students 中按 id 找名字；找不到返回空串而不是抛异常 —— 报告类场景不应因孤儿成绩崩溃。
std::string findStudentName(const std::vector<Student>& students, const std::string& id) {
    auto it = std::find_if(students.begin(), students.end(),
        [&](const Student& s) { return s.getId() == id; });
    return (it == students.end()) ? std::string{} : it->getName();
}

std::string findCourseName(const std::vector<Course>& courses, const std::string& id) {
    auto it = std::find_if(courses.begin(), courses.end(),
        [&](const Course& c) { return c.getCourseId() == id; });
    return (it == courses.end()) ? std::string{} : it->getCourseName();
}

double findCourseCredit(const std::vector<Course>& courses, const std::string& id) {
    auto it = std::find_if(courses.begin(), courses.end(),
        [&](const Course& c) { return c.getCourseId() == id; });
    return (it == courses.end()) ? 0.0 : it->getCredit();
}

} // namespace

GpaResult StatsService::computeGpaFor(const Session& session, const std::string& studentId) {
    requireLoggedIn(session, "Stats.computeGpaFor");
    if (session.isTeacher()) {
        throw PermissionException("Teacher is not allowed to query student GPA");
    }
    if (session.isStudent() && studentId != session.getOwnerId()) {
        throw PermissionException("Student can only query own GPA");
    }

    auto students = studentRepo_.loadAll();
    auto courses  = courseRepo_.loadAll();
    auto scores   = scoreRepo_.loadAll();

    GpaResult result;
    result.studentId   = studentId;
    result.studentName = findStudentName(students, studentId);

    double weightedSum = 0.0;
    for (const auto& s : scores) {
        if (s.getStudentId() != studentId) continue;
        const double credit = findCourseCredit(courses, s.getCourseId());
        if (credit <= 0.0) continue;
        weightedSum += gpaPointFor(s.getTotalScore()) * credit;
        result.totalCredit += credit;
        ++result.courseCount;
    }
    result.gpa = (result.totalCredit > 0.0) ? (weightedSum / result.totalCredit) : 0.0;
    return result;
}

CourseStats StatsService::computeCourseStats(const Session& session, const std::string& courseId) {
    requireLoggedIn(session, "Stats.computeCourseStats");
    if (session.isStudent()) {
        throw PermissionException("Student is not allowed to query course stats");
    }

    auto courses = courseRepo_.loadAll();
    auto courseIt = std::find_if(courses.begin(), courses.end(),
        [&](const Course& c) { return c.getCourseId() == courseId; });
    if (courseIt == courses.end()) {
        throw ValidationException("Course not found: " + courseId);
    }
    if (session.isTeacher() && courseIt->getTeacherId() != session.getOwnerId()) {
        throw PermissionException("Teacher can only query own course stats");
    }

    auto scores = scoreRepo_.loadAll();
    CourseStats stats;
    stats.courseId   = courseId;
    stats.courseName = courseIt->getCourseName();

    double sum = 0.0;
    bool firstHit = true;
    std::size_t passCount = 0;
    std::size_t excellentCount = 0;
    for (const auto& s : scores) {
        if (s.getCourseId() != courseId) continue;
        const double t = s.getTotalScore();
        if (firstHit) {
            stats.max = stats.min = t;
            firstHit = false;
        } else {
            stats.max = std::max(stats.max, t);
            stats.min = std::min(stats.min, t);
        }
        sum += t;
        if (t >= SCORE_PASS)      ++passCount;
        if (t >= SCORE_EXCELLENT) ++excellentCount;
        ++stats.count;
    }
    if (stats.count > 0) {
        stats.avg           = sum / static_cast<double>(stats.count);
        stats.passRate      = static_cast<double>(passCount)      / static_cast<double>(stats.count);
        stats.excellentRate = static_cast<double>(excellentCount) / static_cast<double>(stats.count);
    }
    return stats;
}

std::vector<RankEntry> StatsService::rankByCourse(const Session& session, const std::string& courseId) {
    requireLoggedIn(session, "Stats.rankByCourse");
    if (session.isStudent()) {
        throw PermissionException("Student is not allowed to query course ranking");
    }

    auto courses = courseRepo_.loadAll();
    auto courseIt = std::find_if(courses.begin(), courses.end(),
        [&](const Course& c) { return c.getCourseId() == courseId; });
    if (courseIt == courses.end()) {
        throw ValidationException("Course not found: " + courseId);
    }
    if (session.isTeacher() && courseIt->getTeacherId() != session.getOwnerId()) {
        throw PermissionException("Teacher can only rank own course");
    }

    auto students = studentRepo_.loadAll();
    auto scores   = scoreRepo_.loadAll();

    std::vector<RankEntry> out;
    out.reserve(scores.size());
    for (const auto& s : scores) {
        if (s.getCourseId() != courseId) continue;
        RankEntry e;
        e.studentId   = s.getStudentId();
        e.studentName = findStudentName(students, s.getStudentId());
        e.totalScore  = s.getTotalScore();
        out.push_back(std::move(e));
    }
    std::sort(out.begin(), out.end(),
        [](const RankEntry& a, const RankEntry& b) { return a.totalScore > b.totalScore; });
    return out;
}

std::vector<WarningEntry> StatsService::computeAllWarnings(const Session& session) {
    requireLoggedIn(session, "Stats.computeAllWarnings");
    if (!session.isAdmin()) {
        throw PermissionException("Admin role required for global warning report");
    }

    auto students = studentRepo_.loadAll();
    auto courses  = courseRepo_.loadAll();
    auto scores   = scoreRepo_.loadAll();

    // 按 studentId 聚合一次，避免对每个学生反扫一遍 scores。
    struct Agg {
        double weightedSum = 0.0;
        double totalCredit = 0.0;
        int    failCount = 0;
        std::vector<std::string> failedCourseIds;
    };
    std::unordered_map<std::string, Agg> agg;
    for (const auto& s : scores) {
        const double credit = findCourseCredit(courses, s.getCourseId());
        auto& a = agg[s.getStudentId()];
        if (credit > 0.0) {
            a.weightedSum += gpaPointFor(s.getTotalScore()) * credit;
            a.totalCredit += credit;
        }
        if (s.getTotalScore() < SCORE_PASS) {
            ++a.failCount;
            a.failedCourseIds.push_back(s.getCourseId());
        }
    }

    std::vector<WarningEntry> out;
    for (const auto& stu : students) {
        auto it = agg.find(stu.getId());
        if (it == agg.end()) continue;
        const double gpa = (it->second.totalCredit > 0.0)
            ? (it->second.weightedSum / it->second.totalCredit) : 0.0;
        const bool warn = (gpa < kWarnGpaThreshold) || (it->second.failCount >= kWarnFailThreshold);
        if (!warn) continue;

        WarningEntry e;
        e.studentId       = stu.getId();
        e.studentName     = stu.getName();
        e.gpa             = gpa;
        e.failCount       = it->second.failCount;
        e.failedCourseIds = it->second.failedCourseIds;
        out.push_back(std::move(e));
    }
    std::sort(out.begin(), out.end(),
        [](const WarningEntry& a, const WarningEntry& b) {
            if (a.gpa != b.gpa) return a.gpa < b.gpa;
            return a.studentId < b.studentId;
        });
    return out;
}

} // namespace EduSys
