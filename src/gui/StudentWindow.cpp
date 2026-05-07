#include "EduSys/gui/StudentWindow.hpp"

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

StudentWindow::StudentWindow(AppContext& appContext, Session session, QWidget* parent)
    : QMainWindow(parent)
    , appContext_(appContext)
    , session_(std::move(session)) {
    setWindowTitle(QString::fromUtf8(u8"EduSys 学生端"));
    resize(900, 640);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(createPage(
                     QString::fromUtf8(u8"我的资料"),
                     QString::fromUtf8(u8"后续会接入当前学生本人的资料展示。"),
                     tabs),
                 QString::fromUtf8(u8"我的资料"));
    tabs->addTab(createPage(
                     QString::fromUtf8(u8"我的成绩"),
                     QString::fromUtf8(u8"后续会接入当前学生本人的成绩表格。"),
                     tabs),
                 QString::fromUtf8(u8"我的成绩"));
    tabs->addTab(createPage(
                     QString::fromUtf8(u8"我的 GPA"),
                     QString::fromUtf8(u8"后续会接入 GPA 计算结果与学业概览。"),
                     tabs),
                 QString::fromUtf8(u8"我的 GPA"));
    tabs->addTab(createPage(
                     QString::fromUtf8(u8"账户"),
                     QString::fromUtf8(u8"后续会接入改密与退出登录流程。"),
                     tabs),
                 QString::fromUtf8(u8"账户"));

    setCentralWidget(tabs);
    statusBar()->showMessage(
        QString::fromUtf8(u8"当前学生：%1，GUI 读本人数据的页面骨架已创建。")
            .arg(QString::fromStdString(session_.getUsername())));
}

} // namespace EduSys
