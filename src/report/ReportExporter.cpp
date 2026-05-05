#include "EduSys/report/ReportExporter.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "EduSys/common/Constants.hpp"
#include "EduSys/common/Exception.hpp"
#include "EduSys/common/Logger.hpp"
#include "EduSys/service/Session.hpp"
#include "EduSys/service/StatsService.hpp"

namespace EduSys {

namespace {

// 固定路径；随 data/ 目录寿命一致，单平台课程项目没必要参数化。
constexpr const char* kWarningReportPath = "data/warning_report.txt";

std::string nowTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto tt  = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &tt);
#else
    localtime_r(&tt, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string joinCourseIds(const std::vector<std::string>& ids) {
    std::string out;
    for (std::size_t i = 0; i < ids.size(); ++i) {
        if (i) out.push_back(',');
        out += ids[i];
    }
    return out;
}

// CSV 字段转义（RFC 4180 子集）：含逗号 / 引号 / 换行的字段必须用双引号包裹，
// 内部双引号翻倍。普通 ASCII 字段不动。
// 课程项目数据基本没有逗号，但留这一道防线避免课程名意外含逗号导致解析错位。
std::string csvEscape(const std::string& field) {
    bool needsQuoting = false;
    for (char c : field) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            needsQuoting = true;
            break;
        }
    }
    if (!needsQuoting) return field;
    std::string out;
    out.reserve(field.size() + 2);
    out.push_back('"');
    for (char c : field) {
        if (c == '"') out.push_back('"');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

// 把 double 写成固定 2 位小数，跟 .txt 报告一致。
std::string fmt2(double v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << v;
    return oss.str();
}

// 校验 courseId 不含路径分隔符 / 反斜杠 / 引号等会污染输出文件名的字符；
// 单平台课程项目不需要 OS 级 sanitize，但拒绝明显异常 id 让错误尽早暴露。
void requireSafeCourseIdForFilename(const std::string& courseId) {
    if (courseId.empty()) {
        throw ValidationException("CSV export: courseId must not be empty");
    }
    for (char c : courseId) {
        if (c == '/' || c == '\\' || c == '"' || c == '\'' ||
            c == ':' || c == '*'  || c == '?' || c == '<'  ||
            c == '>' || c == '|'  || c <  0x20) {
            throw ValidationException(
                "CSV export: courseId contains unsafe character: " + courseId);
        }
    }
}

} // namespace

std::string ReportExporter::exportWarningReport(const Session& session) {
    // 先拿数据（失败就抛，文件不会被打开 / 覆盖）
    auto warnings = stats_.computeAllWarnings(session);

    std::ofstream out(kWarningReportPath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        throw StorageException(std::string("Failed to open warning report: ") + kWarningReportPath);
    }

    out << "============================================================\n"
        << " EduSys Academic Warning Report\n"
        << " Generated at : " << nowTimestamp() << "\n"
        << " Operator     : " << session.getUsername() << " (Admin)\n"
        << " Threshold    : GPA < " << std::fixed << std::setprecision(1) << kWarnGpaThreshold
        << "  OR  failCount >= " << kWarnFailThreshold << "\n"
        << " Hits         : " << warnings.size() << "\n"
        << "============================================================\n";

    if (warnings.empty()) {
        out << "\n(No students meet the warning threshold.)\n";
    } else {
        out << "\n"
            << std::left  << std::setw(8)  << "StuId"
            << std::left  << std::setw(14) << "Name"
            << std::right << std::setw(8)  << "GPA"
            << std::right << std::setw(6)  << "Fail"
            << "  " << std::left  << "FailedCourses"
            << "\n"
            << "------------------------------------------------------------\n";
        for (const auto& w : warnings) {
            out << std::left  << std::setw(8)  << w.studentId
                << std::left  << std::setw(14) << w.studentName
                << std::right << std::setw(8)  << std::fixed << std::setprecision(2) << w.gpa
                << std::right << std::setw(6)  << w.failCount
                << "  " << std::left  << joinCourseIds(w.failedCourseIds)
                << "\n";
        }
    }
    out << "\n--- end of report ---\n";
    out.flush();

    Logger::instance().info("Warning report exported: hits=" + std::to_string(warnings.size())
                            + " path=" + kWarningReportPath);
    return kWarningReportPath;
}

// Week 14：课程统计 CSV 导出。复用 StatsService.computeCourseStats，不改任何统计口径。
// 默认仅 Admin 可调用；额外加显式 isAdmin 检查，跟 warning_report 同一边界。
std::string ReportExporter::exportCourseStatsCsv(const Session& session,
                                                 const std::string& courseId) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=ReportExporter.exportCourseStatsCsv)");
    }
    if (!session.isAdmin()) {
        throw PermissionException("Admin role required for CSV export");
    }
    requireSafeCourseIdForFilename(courseId);

    // 数据先拿到再开文件，失败时不会留半残文件。
    const CourseStats stats = stats_.computeCourseStats(session, courseId);

    const std::string path = std::string("data/course_stats_") + courseId + ".csv";
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        throw StorageException("Failed to open CSV: " + path);
    }

    // 表头与 README 对齐；PassRatePct / ExcellentRatePct 按百分数（×100）写入，
    // 跟 .txt 风格一致；CSV 通行做法是带 "Pct" 后缀提示数值含义。
    out << "CourseId,CourseName,Count,Avg,Max,Min,PassRatePct,ExcellentRatePct\n";
    out << csvEscape(stats.courseId)   << ','
        << csvEscape(stats.courseName) << ','
        << stats.count                 << ','
        << fmt2(stats.avg)             << ','
        << fmt2(stats.max)             << ','
        << fmt2(stats.min)             << ','
        << fmt2(stats.passRate * 100.0)      << ','
        << fmt2(stats.excellentRate * 100.0) << '\n';
    out.flush();

    Logger::instance().info("CourseStats CSV exported: courseId=" + courseId + " path=" + path);
    return path;
}

// Week 14：排名 CSV 导出。复用 StatsService.rankByCourse，rank 顺序即返回顺序。
std::string ReportExporter::exportRankingCsv(const Session& session,
                                             const std::string& courseId) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=ReportExporter.exportRankingCsv)");
    }
    if (!session.isAdmin()) {
        throw PermissionException("Admin role required for CSV export");
    }
    requireSafeCourseIdForFilename(courseId);

    const auto ranking = stats_.rankByCourse(session, courseId);

    const std::string path = std::string("data/ranking_") + courseId + ".csv";
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        throw StorageException("Failed to open CSV: " + path);
    }

    out << "Rank,StudentId,StudentName,TotalScore\n";
    int rank = 1;
    for (const auto& r : ranking) {
        out << rank++                       << ','
            << csvEscape(r.studentId)       << ','
            << csvEscape(r.studentName)     << ','
            << fmt2(r.totalScore)           << '\n';
    }
    out.flush();

    Logger::instance().info("Ranking CSV exported: courseId=" + courseId
                            + " rows=" + std::to_string(ranking.size())
                            + " path=" + path);
    return path;
}

} // namespace EduSys
