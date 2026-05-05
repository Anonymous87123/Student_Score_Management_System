#pragma once

#include <string>

namespace EduSys {

class Session;
class StatsService;

// 学业预警报告导出器：唯一一个允许把 StatsService 的结构化结果落盘成文本的组件。
//
// 故意放在独立的 report/ 子目录而不是 service/，强调它的层次：
//   - 不是 Service：不做业务规则 / 权限分支（权限由所传入的 StatsService 内部检查）；
//   - 不是 View：不读用户输入、不打印到控制台；
//   - 不是 Storage：写的是文本预警报告 .txt，而不是结构化的 .dat。
// 它就是一个轻量"格式化 + 写文件"适配器，对 View 暴露 std::string 路径作为执行结果。
//
// Week 14 扩展：同一适配器新增 CSV 导出（仅导出、不导入），复用 StatsService 结果，
// 不改任何统计口径。CSV 路径 / 分隔符 / 表头与 README 文档对齐。
class ReportExporter {
public:
    explicit ReportExporter(StatsService& stats) : stats_(stats) {}

    // 触发 StatsService::computeAllWarnings -> 渲染固定格式 -> 写到 data/warning_report.txt。
    // 仅 Admin 可调用；权限失败一律抛 PermissionException（来自 StatsService）。
    // 返回写入的文件绝对/相对路径，供 View 在控制台回显。
    std::string exportWarningReport(const Session& session);

    // Week 14 CSV 导出（仅 Admin）：路径 data/course_stats_<courseId>.csv。
    // 数据源：StatsService::computeCourseStats(session, courseId)。
    // 表头：CourseId,CourseName,Count,Avg,Max,Min,PassRatePct,ExcellentRatePct
    std::string exportCourseStatsCsv(const Session& session, const std::string& courseId);

    // Week 14 CSV 导出（仅 Admin）：路径 data/ranking_<courseId>.csv。
    // 数据源：StatsService::rankByCourse(session, courseId)。
    // 表头：Rank,StudentId,StudentName,TotalScore
    std::string exportRankingCsv(const Session& session, const std::string& courseId);

private:
    StatsService& stats_;
};

} // namespace EduSys
