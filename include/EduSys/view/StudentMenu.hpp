#pragma once

#include <string>
#include <vector>

#include "EduSys/view/BaseMenu.hpp"

namespace EduSys {

class Score;
class Session;
class AuthService;
class StudentService;
class CourseService;
class ScoreService;
class StatsService;

// 学生菜单：纯只读自己的资料 + GPA + 成绩；唯一会修改持久化的动作是改密。
class StudentMenu : public BaseMenu {
public:
    StudentMenu(Session&         session,
                AuthService&     authSvc,
                StudentService&  studentSvc,
                CourseService&   courseSvc,
                ScoreService&    scoreSvc,
                StatsService&    statsSvc)
        : session_(session), authSvc_(authSvc),
          studentSvc_(studentSvc), courseSvc_(courseSvc),
          scoreSvc_(scoreSvc), statsSvc_(statsSvc) {}

    void run() override;

private:
    void viewProfile();
    void viewMyCourses();
    void viewMyScores();
    void viewMyScoresBySemester();
    void viewMyScoresByCourse();
    void viewMyScoresByCourseAndSemester();
    void viewScoreChart();
    void viewMyGpa();
    void changePassword();
    void printStudentScoreRows(const std::vector<Score>& list, const std::string& emptyText);

    Session&         session_;
    AuthService&     authSvc_;
    StudentService&  studentSvc_;
    CourseService&   courseSvc_;
    ScoreService&    scoreSvc_;
    StatsService&    statsSvc_;
};

} // namespace EduSys
