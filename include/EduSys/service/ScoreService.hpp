#pragma once

#include <string>
#include <vector>

#include "EduSys/model/Score.hpp"
#include "EduSys/storage/CourseRepository.hpp"
#include "EduSys/storage/ScoreRepository.hpp"
#include "EduSys/storage/StudentRepository.hpp"

namespace EduSys {

class Session;

// 成绩服务：所有成绩相关业务规则的唯一入口。
// 权限矩阵：
//   Admin   : 全部读写
//   Teacher : 只读自己授课的成绩；只写自己授课的成绩（Course.teacherId == session.ownerId）
//   Student : 只读自己的成绩；禁止写
// 唯一键：(studentId, courseId, semester)；upsert 同键覆写、不同键追加。
// 范围校验：usual/final/total ∈ [SCORE_MIN, SCORE_MAX]；studentId / courseId 必须存在。
class ScoreService {
public:
    ScoreService(ScoreRepository&   scoreRepo,
                 CourseRepository&  courseRepo,
                 StudentRepository& studentRepo)
        : scoreRepo_(scoreRepo), courseRepo_(courseRepo), studentRepo_(studentRepo) {}

    std::vector<Score> listAll(const Session& session);
    std::vector<Score> findByStudent(const Session& session, const std::string& studentId);
    std::vector<Score> findByCourse(const Session& session, const std::string& courseId);

    // upsert：同 (studentId, courseId, semester) 已存在 -> 覆写；否则追加。
    void upsert(const Session& session, const Score& score);

    void remove(const Session& session,
                const std::string& studentId,
                const std::string& courseId,
                const std::string& semester);

private:
    ScoreRepository&   scoreRepo_;
    CourseRepository&  courseRepo_;
    StudentRepository& studentRepo_;
};

} // namespace EduSys
