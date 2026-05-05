#pragma once

#include <string>
#include <vector>

#include "EduSys/model/Student.hpp"
#include "EduSys/storage/ScoreRepository.hpp"
#include "EduSys/storage/StudentRepository.hpp"
#include "EduSys/storage/UserRepository.hpp"

namespace EduSys {

class Session;

// 学生服务：所有学生相关业务规则的唯一入口。
// 权限矩阵：
//   Admin   : 全部读写
//   Teacher : 只读
//   Student : 只能读自己（id == session.ownerId）
// remove() 物理级联：先清 scores.dat 中相关记录，再删 users.dat 中 ownerId 匹配的学生账号，
// 最后删除 students.dat 中的学生本人；任一步失败立即抛异常，不做"半成功"持久化。
class StudentService {
public:
    StudentService(StudentRepository& studentRepo,
                   ScoreRepository&   scoreRepo,
                   UserRepository&    userRepo)
        : studentRepo_(studentRepo), scoreRepo_(scoreRepo), userRepo_(userRepo) {}

    std::vector<Student> listAll(const Session& session);
    Student              findById(const Session& session, const std::string& id);

    void create(const Session& session, const Student& student);
    void update(const Session& session, const Student& student);
    void remove(const Session& session, const std::string& id);

private:
    StudentRepository& studentRepo_;
    ScoreRepository&   scoreRepo_;
    UserRepository&    userRepo_;
};

} // namespace EduSys
