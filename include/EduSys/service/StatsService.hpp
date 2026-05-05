#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "EduSys/storage/CourseRepository.hpp"
#include "EduSys/storage/ScoreRepository.hpp"
#include "EduSys/storage/StudentRepository.hpp"

namespace EduSys {

class Session;

// 单个学生的 GPA 结果（4.0 制分段加权平均，按课程学分加权）。
struct GpaResult {
    std::string studentId;
    std::string studentName;
    int         courseCount = 0;
    double      totalCredit = 0.0;
    double      gpa         = 0.0;   // 0.0 .. 4.0
};

// 某课程下的成绩分布统计。
struct CourseStats {
    std::string courseId;
    std::string courseName;
    std::size_t count         = 0;
    double      avg           = 0.0;
    double      max           = 0.0;
    double      min           = 0.0;
    double      passRate      = 0.0;   // [0,1]
    double      excellentRate = 0.0;   // [0,1]
};

// 某课程的学生总评排名。
struct RankEntry {
    std::string studentId;
    std::string studentName;
    double      totalScore = 0.0;
};

// 学业预警条目：GPA 低于阈值，或挂科门数达到阈值。
struct WarningEntry {
    std::string              studentId;
    std::string              studentName;
    double                   gpa = 0.0;
    int                      failCount = 0;
    std::vector<std::string> failedCourseIds;
};

// 学业预警阈值：GPA < kWarnGpaThreshold  OR  failCount >= kWarnFailThreshold。
inline constexpr double kWarnGpaThreshold  = 2.0;
inline constexpr int    kWarnFailThreshold = 2;

// 统计服务：只返回结构化结果，不承担任何文本排版 / 文件 I/O。
// 权限矩阵：
//   computeGpaFor       : Admin 任意 / Student 仅自己 / Teacher 拒绝
//   computeCourseStats  : Admin 任意 / Teacher 仅自己授课 / Student 拒绝
//   rankByCourse        : 同上
//   computeAllWarnings  : 仅 Admin
class StatsService {
public:
    StatsService(StudentRepository& studentRepo,
                 CourseRepository&  courseRepo,
                 ScoreRepository&   scoreRepo)
        : studentRepo_(studentRepo), courseRepo_(courseRepo), scoreRepo_(scoreRepo) {}

    GpaResult                 computeGpaFor(const Session& session, const std::string& studentId);
    CourseStats               computeCourseStats(const Session& session, const std::string& courseId);
    std::vector<RankEntry>    rankByCourse(const Session& session, const std::string& courseId);
    std::vector<WarningEntry> computeAllWarnings(const Session& session);

private:
    StudentRepository& studentRepo_;
    CourseRepository&  courseRepo_;
    ScoreRepository&   scoreRepo_;
};

} // namespace EduSys
