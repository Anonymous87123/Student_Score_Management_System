#pragma once

#include <string>
#include <vector>

#include "EduSys/model/Course.hpp"
#include "EduSys/storage/CourseRepository.hpp"
#include "EduSys/storage/ScoreRepository.hpp"
#include "EduSys/storage/TeacherRepository.hpp"

namespace EduSys {

class Session;

// 课程服务：所有课程相关业务规则的唯一入口。
// 权限矩阵：
//   Admin   : 全部读写
//   Teacher : 只读
//   Student : 只读
// remove() 物理级联：先清 scores.dat 中相关记录，再删 courses.dat 中的课程本身。
// create / update 时校验 teacherId 必须在 teachers.dat 中存在。
class CourseService {
public:
    CourseService(CourseRepository&  courseRepo,
                  ScoreRepository&   scoreRepo,
                  TeacherRepository& teacherRepo)
        : courseRepo_(courseRepo), scoreRepo_(scoreRepo), teacherRepo_(teacherRepo) {}

    std::vector<Course> listAll(const Session& session);
    Course              findById(const Session& session, const std::string& courseId);

    void create(const Session& session, const Course& course);
    void update(const Session& session, const Course& course);
    void remove(const Session& session, const std::string& courseId);

private:
    CourseRepository&  courseRepo_;
    ScoreRepository&   scoreRepo_;
    TeacherRepository& teacherRepo_;
};

} // namespace EduSys
