#include <exception>
#include <memory>
#include <string>

#include <QApplication>
#include <QDialog>
#include <QMainWindow>
#include <QMessageBox>

#include "EduSys/app/AppContext.hpp"
#include "EduSys/common/Exception.hpp"
#include "EduSys/common/Logger.hpp"
#include "EduSys/common/Types.hpp"
#include "EduSys/gui/AdminWindow.hpp"
#include "EduSys/gui/LoginDialog.hpp"
#include "EduSys/gui/StudentWindow.hpp"
#include "EduSys/gui/TeacherWindow.hpp"

namespace {

QString errorText(const std::exception& e) {
    return QString::fromLocal8Bit(e.what());
}

std::unique_ptr<QMainWindow> createRoleWindow(EduSys::AppContext& appContext,
                                              const EduSys::Session& session) {
    using namespace EduSys;

    switch (session.getRole()) {
        case RoleType::Admin:
            return std::make_unique<AdminWindow>(appContext, session);
        case RoleType::Teacher:
            return std::make_unique<TeacherWindow>(appContext, session);
        case RoleType::Student:
            return std::make_unique<StudentWindow>(appContext, session);
    }

    throw EduSys::EduException("Unsupported session role.");
}

} // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QString::fromUtf8(u8"EduSys GUI"));
    QApplication::setOrganizationName("EduSys");

    try {
        auto& logger = EduSys::Logger::instance();
        logger.info("EduSys GUI starting up.");

        EduSys::AppContext appContext;
        const bool seededThisRun = appContext.initializeData();

        if (seededThisRun) {
            QMessageBox::information(
                nullptr,
                QString::fromUtf8(u8"示例数据已初始化"),
                QString::fromUtf8(
                    u8"检测到当前仓储为空，系统已写入默认示例账号：\n"
                    u8"管理员：admin / admin123\n"
                    u8"教师：t001 / t001pw\n"
                    u8"学生：s001 / s001pw"));
        }

        EduSys::LoginDialog loginDialog(appContext);
        if (loginDialog.exec() != QDialog::Accepted) {
            logger.info("EduSys GUI shutdown normally (login cancelled).");
            return 0;
        }

        auto mainWindow = createRoleWindow(appContext, loginDialog.session());
        mainWindow->show();

        const int exitCode = app.exec();
        logger.info("EduSys GUI shutdown normally.");
        return exitCode;
    } catch (const std::exception& e) {
        QMessageBox::critical(
            nullptr,
            QString::fromUtf8(u8"启动失败"),
            errorText(e));
        try {
            EduSys::Logger::instance().error(std::string("GUI fatal: ") + e.what());
        } catch (...) {
            // Swallow secondary logging failures during fatal shutdown.
        }
        return 1;
    }
}
