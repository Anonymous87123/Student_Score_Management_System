#pragma once

namespace EduSys {

// 数据文件相对路径（运行目录下的 data/）。
inline constexpr const char* USERS_FILE    = "data/users.dat";
inline constexpr const char* STUDENTS_FILE = "data/students.dat";
inline constexpr const char* TEACHERS_FILE = "data/teachers.dat";
inline constexpr const char* COURSES_FILE  = "data/courses.dat";
inline constexpr const char* SCORES_FILE   = "data/scores.dat";
inline constexpr const char* LOG_FILE      = "data/app.log";

inline constexpr const char* DATA_DIR = "data";

// 成绩相关阈值。
inline constexpr double SCORE_MIN       = 0.0;
inline constexpr double SCORE_MAX       = 100.0;
inline constexpr double SCORE_PASS      = 60.0;
inline constexpr double SCORE_EXCELLENT = 90.0;

// 总评默认权重（平时 30% + 期末 70%）。
inline constexpr double USUAL_WEIGHT = 0.3;
inline constexpr double FINAL_WEIGHT = 0.7;

} // namespace EduSys
