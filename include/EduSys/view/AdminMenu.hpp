#pragma once

#include "EduSys/view/BaseMenu.hpp"

namespace EduSys {

class Session;
class AuthService;
class StudentService;
class CourseService;
class ScoreService;
class StatsService;
class ReportExporter;

// 管理员主菜单：全量 CRUD + 统计 + 预警报告导出 + 改密。
// 只持有 Session 与 Service / Exporter 引用；不直接触碰 Repository。
class AdminMenu : public BaseMenu {
public:
    AdminMenu(Session&         session,
              AuthService&     authSvc,
              StudentService&  studentSvc,
              CourseService&   courseSvc,
              ScoreService&    scoreSvc,
              StatsService&    statsSvc,
              ReportExporter&  reportExporter)
        : session_(session), authSvc_(authSvc),
          studentSvc_(studentSvc), courseSvc_(courseSvc), scoreSvc_(scoreSvc),
          statsSvc_(statsSvc), reportExporter_(reportExporter) {}

    void run() override;

private:
    void studentMenu();
    void courseMenu();
    void scoreMenu();
    void statsMenu();
    void generateWarningReport();
    void exportCsv();
    void changePassword();

    Session&         session_;
    AuthService&     authSvc_;
    StudentService&  studentSvc_;
    CourseService&   courseSvc_;
    ScoreService&    scoreSvc_;
    StatsService&    statsSvc_;
    ReportExporter&  reportExporter_;
};

} // namespace EduSys
