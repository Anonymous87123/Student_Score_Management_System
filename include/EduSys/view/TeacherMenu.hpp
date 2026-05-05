#pragma once

#include "EduSys/view/BaseMenu.hpp"

namespace EduSys {

class Session;
class AuthService;
class CourseService;
class ScoreService;
class StatsService;

// 教师菜单：所有"录分 / 改分 / 删分 / 课程统计"路径都先按 teacherId 严格过滤。
// 即使 Service 内部已经做权限拒绝，View 层这里仍主动按 session.ownerId 拉一份"我的授课课程"，
// 用户选课程时只能在这份白名单里选，避免输错 courseId 后被 Service 反弹。
class TeacherMenu : public BaseMenu {
public:
    TeacherMenu(Session&        session,
                AuthService&    authSvc,
                CourseService&  courseSvc,
                ScoreService&   scoreSvc,
                StatsService&   statsSvc)
        : session_(session), authSvc_(authSvc),
          courseSvc_(courseSvc), scoreSvc_(scoreSvc), statsSvc_(statsSvc) {}

    void run() override;

private:
    void listMyCourses();
    void viewMyScores();
    void upsertScore();
    void deleteScore();
    void courseStats();
    void changePassword();

    // 选一门"我授课"的课程；返回空串表示用户取消或没有授课。
    std::string pickOwnCourseId();

    Session&        session_;
    AuthService&    authSvc_;
    CourseService&  courseSvc_;
    ScoreService&   scoreSvc_;
    StatsService&   statsSvc_;
};

} // namespace EduSys
