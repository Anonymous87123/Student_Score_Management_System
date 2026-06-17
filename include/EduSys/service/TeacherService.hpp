#pragma once

#include <string>
#include <vector>

#include "EduSys/model/Teacher.hpp"
#include "EduSys/storage/CourseRepository.hpp"
#include "EduSys/storage/ScoreRepository.hpp"
#include "EduSys/storage/TeacherRepository.hpp"
#include "EduSys/storage/UserRepository.hpp"

namespace EduSys {

class Session;

// 教师服务：管理员维护教师资料；删除教师时先从授课课程中移除该教师，
// 若课程已无任何授课教师，才同步删除课程及其成绩；教师账号始终一并清理。
class TeacherService {
public:
    TeacherService(TeacherRepository& teacherRepo,
                   CourseRepository&  courseRepo,
                   ScoreRepository&   scoreRepo,
                   UserRepository&    userRepo)
        : teacherRepo_(teacherRepo)
        , courseRepo_(courseRepo)
        , scoreRepo_(scoreRepo)
        , userRepo_(userRepo) {}

    std::vector<Teacher> listAll(const Session& session);
    Teacher              findById(const Session& session, const std::string& id);

    void create(const Session& session,
                const Teacher& teacher,
                const std::string& username = {},
                const std::string& plainPassword = {});
    void update(const Session& session, const Teacher& teacher);
    void remove(const Session& session, const std::string& id);

private:
    TeacherRepository& teacherRepo_;
    CourseRepository&  courseRepo_;
    ScoreRepository&   scoreRepo_;
    UserRepository&    userRepo_;
};

} // namespace EduSys
