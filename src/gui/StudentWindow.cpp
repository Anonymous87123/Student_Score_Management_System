#include "EduSys/gui/StudentWindow.hpp"

#include <utility>
#include <vector>

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "EduSys/app/AppContext.hpp"
#include "EduSys/gui/ChangePasswordDialog.hpp"
#include "EduSys/model/Score.hpp"
#include "EduSys/model/Student.hpp"
#include "EduSys/service/StatsService.hpp"

namespace {

QTableWidgetItem* createReadOnlyItem(const QString& text) {
    auto* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

void configureTable(QTableWidget* table) {
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(false);
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
    tabs->addTab(createProfilePage(), QString::fromUtf8(u8"我的资料"));
    tabs->addTab(createMyScoresPage(), QString::fromUtf8(u8"我的成绩"));
    tabs->addTab(createMyGpaPage(), QString::fromUtf8(u8"我的 GPA"));
    tabs->addTab(createAccountPage(), QString::fromUtf8(u8"账户"));

    setCentralWidget(tabs);
    statusBar()->showMessage(
        QString::fromUtf8(u8"当前学生：%1")
            .arg(QString::fromStdString(session_.getUsername())));
}

// ---------------------------------------------------------------------------
// Profile page
// ---------------------------------------------------------------------------

QWidget* StudentWindow::createProfilePage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>我的资料</b>"), page);
    rootLayout->addWidget(titleLabel);

    try {
        const Student me = appContext_.studentService.findById(session_, session_.getOwnerId());
        auto* infoLabel = new QLabel(
            QString::fromUtf8(
                u8"学号：%1\n姓名：%2\n专业：%3\n班级：%4\n入学年份：%5\n联系方式：%6")
                    .arg(QString::fromStdString(me.getId()))
                    .arg(QString::fromStdString(me.getName()))
                    .arg(QString::fromStdString(me.getMajor()))
                    .arg(QString::fromStdString(me.getClassName()))
                    .arg(me.getEnrollYear())
                    .arg(QString::fromStdString(me.getContact())),
            page);
        infoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        rootLayout->addWidget(infoLabel);
    } catch (const std::exception& e) {
        rootLayout->addWidget(new QLabel(
            QString::fromUtf8(u8"加载资料失败：%1").arg(QString::fromLocal8Bit(e.what())), page));
    }

    rootLayout->addStretch();
    return page;
}

// ---------------------------------------------------------------------------
// My scores page
// ---------------------------------------------------------------------------

QWidget* StudentWindow::createMyScoresPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>我的成绩</b>"), page);
    rootLayout->addWidget(titleLabel);

    auto* refreshButton = new QPushButton(QString::fromUtf8(u8"刷新"), page);
    rootLayout->addWidget(refreshButton);

    scoreTable_ = new QTableWidget(page);
    scoreTable_->setColumnCount(5);
    scoreTable_->setHorizontalHeaderLabels({
        QString::fromUtf8(u8"课程号"),
        QString::fromUtf8(u8"学期"),
        QString::fromUtf8(u8"平时"),
        QString::fromUtf8(u8"期末"),
        QString::fromUtf8(u8"总评")
    });
    configureTable(scoreTable_);
    rootLayout->addWidget(scoreTable_, 1);

    connect(refreshButton, &QPushButton::clicked, this, [this] { refreshMyScores(); });

    refreshMyScores();
    return page;
}

void StudentWindow::refreshMyScores() {
    try {
        const auto scores = appContext_.scoreService.findByStudent(session_, session_.getOwnerId());
        scoreTable_->setRowCount(static_cast<int>(scores.size()));

        int row = 0;
        for (const auto& s : scores) {
            scoreTable_->setItem(row, 0, createReadOnlyItem(QString::fromStdString(s.getCourseId())));
            scoreTable_->setItem(row, 1, createReadOnlyItem(QString::fromStdString(s.getSemester())));
            scoreTable_->setItem(row, 2, createReadOnlyItem(QString::number(s.getUsualScore(), 'f', 1)));
            scoreTable_->setItem(row, 3, createReadOnlyItem(QString::number(s.getFinalScore(), 'f', 1)));
            scoreTable_->setItem(row, 4, createReadOnlyItem(QString::number(s.getTotalScore(), 'f', 1)));
            ++row;
        }

        statusBar()->showMessage(
            QString::fromUtf8(u8"我的成绩已刷新，共 %1 条。").arg(scores.size()), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"刷新成绩失败"), e);
    }
}

// ---------------------------------------------------------------------------
// My GPA page
// ---------------------------------------------------------------------------

QWidget* StudentWindow::createMyGpaPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>我的 GPA</b>"), page);
    rootLayout->addWidget(titleLabel);

    gpaLabel_ = new QLabel(page);
    gpaLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    rootLayout->addWidget(gpaLabel_);

    auto* refreshButton = new QPushButton(QString::fromUtf8(u8"刷新 GPA"), page);
    rootLayout->addWidget(refreshButton);

    rootLayout->addStretch();

    connect(refreshButton, &QPushButton::clicked, this, [this] { refreshMyGpa(); });

    refreshMyGpa();
    return page;
}

void StudentWindow::refreshMyGpa() {
    try {
        const GpaResult gpa = appContext_.statsService.computeGpaFor(session_, session_.getOwnerId());
        gpaLabel_->setText(
            QString::fromUtf8(
                u8"学号：%1\n"
                u8"姓名：%2\n"
                u8"已修课程数：%3\n"
                u8"总学分：%4\n"
                u8"GPA：%5")
                    .arg(QString::fromStdString(gpa.studentId))
                    .arg(QString::fromStdString(gpa.studentName))
                    .arg(gpa.courseCount)
                    .arg(gpa.totalCredit, 0, 'f', 1)
                    .arg(gpa.gpa, 0, 'f', 3));
        statusBar()->showMessage(QString::fromUtf8(u8"GPA 已刷新。"), 4000);
    } catch (const std::exception& e) {
        gpaLabel_->setText(QString::fromUtf8(u8"加载 GPA 失败：%1").arg(QString::fromLocal8Bit(e.what())));
    }
}

// ---------------------------------------------------------------------------
// Account page
// ---------------------------------------------------------------------------

QWidget* StudentWindow::createAccountPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>账户</b>"), page);
    auto* infoLabel = new QLabel(
        QString::fromUtf8(u8"当前用户：%1（角色：学生）")
            .arg(QString::fromStdString(session_.getUsername())),
        page);
    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(infoLabel);
    rootLayout->addSpacing(16);

    auto* changePwButton = new QPushButton(QString::fromUtf8(u8"修改密码"), page);
    changePwButton->setMaximumWidth(200);
    rootLayout->addWidget(changePwButton);

    rootLayout->addSpacing(16);

    auto* logoutButton = new QPushButton(QString::fromUtf8(u8"退出登录"), page);
    logoutButton->setMaximumWidth(200);
    rootLayout->addWidget(logoutButton);

    rootLayout->addStretch();

    connect(changePwButton, &QPushButton::clicked, this, [this] {
        ChangePasswordDialog dialog(appContext_, session_, this);
        dialog.exec();
    });

    connect(logoutButton, &QPushButton::clicked, this, [this] {
        const auto confirm = QMessageBox::question(
            this,
            QString::fromUtf8(u8"确认退出"),
            QString::fromUtf8(u8"确定要退出登录吗？"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (confirm == QMessageBox::Yes) {
            close();
        }
    });

    return page;
}

void StudentWindow::showServiceError(const QString& title, const std::exception& e) {
    QMessageBox::critical(this, title, QString::fromLocal8Bit(e.what()));
}

} // namespace EduSys

