#pragma once

#include "EduSys/view/BaseMenu.hpp"

namespace EduSys {

class Session;
class AuthService;
class StudentService;
class ScoreService;
class StatsService;

// 学生菜单：纯只读自己的资料 + GPA + 成绩；唯一会修改持久化的动作是改密。
class StudentMenu : public BaseMenu {
public:
    StudentMenu(Session&         session,
                AuthService&     authSvc,
                StudentService&  studentSvc,
                ScoreService&    scoreSvc,
                StatsService&    statsSvc)
        : session_(session), authSvc_(authSvc),
          studentSvc_(studentSvc), scoreSvc_(scoreSvc), statsSvc_(statsSvc) {}

    void run() override;

private:
    void viewProfile();
    void viewMyScores();
    void viewMyGpa();
    void changePassword();

    Session&         session_;
    AuthService&     authSvc_;
    StudentService&  studentSvc_;
    ScoreService&    scoreSvc_;
    StatsService&    statsSvc_;
};

} // namespace EduSys
