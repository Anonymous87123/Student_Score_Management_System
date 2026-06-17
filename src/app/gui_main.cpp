#include <exception>
#include <memory>
#include <string>

#include <QApplication>
#include <QColor>
#include <QDialog>
#include <QEventLoop>
#include <QMainWindow>
#include <QMessageBox>
#include <QObject>
#include <QPalette>
#include <QStyleFactory>

#include "EduSys/app/AppContext.hpp"
#include "EduSys/common/Exception.hpp"
#include "EduSys/common/Logger.hpp"
#include "EduSys/common/Types.hpp"
#include "EduSys/gui/AdminWindow.hpp"
#include "EduSys/gui/LoginDialog.hpp"
#include "EduSys/gui/StudentWindow.hpp"
#include "EduSys/gui/TeacherWindow.hpp"

namespace {

QPalette lightPalette() {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#ffffff"));
    palette.setColor(QPalette::WindowText, QColor("#0f172a"));
    palette.setColor(QPalette::Base, QColor("#ffffff"));
    palette.setColor(QPalette::AlternateBase, QColor("#f8fbff"));
    palette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
    palette.setColor(QPalette::ToolTipText, QColor("#0f172a"));
    palette.setColor(QPalette::Text, QColor("#0f172a"));
    palette.setColor(QPalette::Button, QColor("#ffffff"));
    palette.setColor(QPalette::ButtonText, QColor("#0f172a"));
    palette.setColor(QPalette::BrightText, QColor("#ffffff"));
    palette.setColor(QPalette::Highlight, QColor("#cfe3ff"));
    palette.setColor(QPalette::HighlightedText, QColor("#0f172a"));
    palette.setColor(QPalette::Link, QColor("#2563eb"));
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    palette.setColor(QPalette::PlaceholderText, QColor("#94a3b8"));
#endif
    return palette;
}

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
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setPalette(lightPalette());
    app.setStyleSheet(QString::fromLatin1(R"(
        QDialog, QMessageBox {
            background-color: #ffffff;
            color: #0f172a;
        }
        QDialog QLabel, QMessageBox QLabel {
            color: #0f172a;
        }
        QDialog QPushButton, QMessageBox QPushButton {
            background-color: #ffffff;
            color: #0f172a;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            min-width: 72px;
            padding: 6px 14px;
        }
        QDialog QPushButton:hover, QMessageBox QPushButton:hover {
            background-color: #f8fbff;
            border-color: #7fb0e9;
        }
        QDialog QPushButton:pressed, QMessageBox QPushButton:pressed {
            background-color: #eaf2ff;
        }
        QDialog QLineEdit, QDialog QComboBox, QDialog QSpinBox, QDialog QDoubleSpinBox,
        QDialog QTextEdit, QDialog QPlainTextEdit {
            background-color: #ffffff;
            color: #0f172a;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            padding: 6px 8px;
            selection-background-color: #bfdbfe;
        }
    )"));

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
                    u8"教师：t001 / t001pw，t002 / t002pw ... t006 / t006pw\n"
                    u8"学生：s001 / s001pw，s003 / s003pw ... s101 / s101pw"));
        }

        while (true) {
            EduSys::LoginDialog loginDialog(appContext);
            if (loginDialog.exec() != QDialog::Accepted) {
                logger.info("EduSys GUI shutdown normally (login cancelled).");
                break;
            }

            auto* mainWindow = createRoleWindow(appContext, loginDialog.session()).release();
            mainWindow->setAttribute(Qt::WA_DeleteOnClose);
            mainWindow->show();

            QEventLoop eventLoop;
            QObject::connect(mainWindow, &QObject::destroyed, &eventLoop, &QEventLoop::quit);
            eventLoop.exec();
        }

        logger.info("EduSys GUI shutdown normally.");
        return 0;
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
