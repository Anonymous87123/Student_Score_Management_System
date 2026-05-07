#include "EduSys/gui/TeacherWindow.hpp"

#include <utility>

#include <QLabel>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "EduSys/app/AppContext.hpp"

namespace {

QWidget* createPage(const QString& title, const QString& summary, QWidget* parent = nullptr) {
    auto* page = new QWidget(parent);
    auto* layout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>%1</b>").arg(title), page);
    auto* summaryLabel = new QLabel(summary, page);
    summaryLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addWidget(summaryLabel);
    layout->addStretch();
    return page;
}

} // namespace

namespace EduSys {

TeacherWindow::TeacherWindow(AppContext& appContext, Session session, QWidget* parent)
    : QMainWindow(parent)
    , appContext_(appContext)
    , session_(std::move(session)) {
    setWindowTitle(QString::fromUtf8(u8"EduSys 教师端"));
    resize(1000, 680);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(createPage(
                     QString::fromUtf8(u8"我的课程"),
                     QString::fromUtf8(u8"后续会只显示当前教师自己的课程白名单。"),
                     tabs),
                 QString::fromUtf8(u8"我的课程"));
    tabs->addTab(createPage(
                     QString::fromUtf8(u8"我的课程成绩"),
                     QString::fromUtf8(u8"后续会接入仅针对本人授课课程的成绩查看、录入、更新与删除。"),
                     tabs),
                 QString::fromUtf8(u8"我的课程成绩"));
    tabs->addTab(createPage(
                     QString::fromUtf8(u8"我的课程统计"),
                     QString::fromUtf8(u8"后续会接入只针对本人课程的统计与排名。"),
                     tabs),
                 QString::fromUtf8(u8"我的课程统计"));
    tabs->addTab(createPage(
                     QString::fromUtf8(u8"账户"),
                     QString::fromUtf8(u8"后续会接入改密与退出登录流程。"),
                     tabs),
                 QString::fromUtf8(u8"账户"));

    setCentralWidget(tabs);
    statusBar()->showMessage(
        QString::fromUtf8(u8"当前教师：%1，课程白名单逻辑将在下一阶段接入。")
            .arg(QString::fromStdString(session_.getUsername())));
}

} // namespace EduSys
