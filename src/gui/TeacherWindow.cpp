#include "EduSys/gui/TeacherWindow.hpp"

#include <utility>
#include <vector>

#include <QAbstractItemView>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
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
#include "EduSys/gui/ScoreEditDialog.hpp"
#include "EduSys/model/Score.hpp"
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

TeacherWindow::TeacherWindow(AppContext& appContext, Session session, QWidget* parent)
    : QMainWindow(parent)
    , appContext_(appContext)
    , session_(std::move(session)) {
    setWindowTitle(QString::fromUtf8(u8"EduSys 教师端"));
    resize(1000, 680);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(createMyCoursesPage(), QString::fromUtf8(u8"我的课程"));
    tabs->addTab(createMyScoresPage(), QString::fromUtf8(u8"我的课程成绩"));
    tabs->addTab(createMyStatsPage(), QString::fromUtf8(u8"我的课程统计"));
    tabs->addTab(createAccountPage(), QString::fromUtf8(u8"账户"));

    setCentralWidget(tabs);
    statusBar()->showMessage(
        QString::fromUtf8(u8"当前教师：%1。教师端全部 4 个页签已接入真实业务层。")
            .arg(QString::fromStdString(session_.getUsername())));
}

std::vector<Course> TeacherWindow::myCourses() {
    return appContext_.courseService.listAll(session_);
}

// ---------------------------------------------------------------------------
// My courses page
// ---------------------------------------------------------------------------

QWidget* TeacherWindow::createMyCoursesPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>我的课程</b>"), page);
    rootLayout->addWidget(titleLabel);

    auto* refreshButton = new QPushButton(QString::fromUtf8(u8"刷新"), page);
    rootLayout->addWidget(refreshButton);

    courseTable_ = new QTableWidget(page);
    courseTable_->setColumnCount(4);
    courseTable_->setHorizontalHeaderLabels({
        QString::fromUtf8(u8"课程号"),
        QString::fromUtf8(u8"课程名"),
        QString::fromUtf8(u8"学分"),
        QString::fromUtf8(u8"学期")
    });
    configureTable(courseTable_);
    rootLayout->addWidget(courseTable_, 1);

    connect(refreshButton, &QPushButton::clicked, this, [this] { refreshMyCourses(); });

    refreshMyCourses();
    return page;
}

void TeacherWindow::refreshMyCourses() {
    try {
        const auto courses = myCourses();
        courseTable_->setRowCount(static_cast<int>(courses.size()));

        int row = 0;
        for (const auto& c : courses) {
            courseTable_->setItem(row, 0, createReadOnlyItem(QString::fromStdString(c.getCourseId())));
            courseTable_->setItem(row, 1, createReadOnlyItem(QString::fromStdString(c.getCourseName())));
            courseTable_->setItem(row, 2, createReadOnlyItem(QString::number(c.getCredit(), 'f', 1)));
            courseTable_->setItem(row, 3, createReadOnlyItem(QString::fromStdString(c.getSemester())));
            ++row;
        }

        statusBar()->showMessage(
            QString::fromUtf8(u8"我的课程已刷新，共 %1 门。").arg(courses.size()), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"刷新我的课程失败"), e);
    }
}

// ---------------------------------------------------------------------------
// My scores page
// ---------------------------------------------------------------------------

QWidget* TeacherWindow::createMyScoresPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>我的课程成绩</b>"), page);
    rootLayout->addWidget(titleLabel);

    auto* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"选择课程"), page));
    scoreCourseCombo_ = new QComboBox(page);
    filterLayout->addWidget(scoreCourseCombo_, 1);
    auto* loadButton = new QPushButton(QString::fromUtf8(u8"加载成绩"), page);
    filterLayout->addWidget(loadButton);
    rootLayout->addLayout(filterLayout);

    auto* actionLayout = new QHBoxLayout();
    auto* createButton = new QPushButton(QString::fromUtf8(u8"录入成绩"), page);
    auto* editButton = new QPushButton(QString::fromUtf8(u8"编辑选中"), page);
    auto* removeButton = new QPushButton(QString::fromUtf8(u8"删除选中"), page);
    actionLayout->addWidget(createButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    rootLayout->addLayout(actionLayout);

    scoreTable_ = new QTableWidget(page);
    scoreTable_->setColumnCount(6);
    scoreTable_->setHorizontalHeaderLabels({
        QString::fromUtf8(u8"学号"),
        QString::fromUtf8(u8"课程号"),
        QString::fromUtf8(u8"学期"),
        QString::fromUtf8(u8"平时"),
        QString::fromUtf8(u8"期末"),
        QString::fromUtf8(u8"总评")
    });
    configureTable(scoreTable_);
    rootLayout->addWidget(scoreTable_, 1);

    connect(loadButton, &QPushButton::clicked, this, [this] { refreshMyScores(); });
    connect(createButton, &QPushButton::clicked, this, [this] { createScore(); });
    connect(editButton, &QPushButton::clicked, this, [this] { editSelectedScore(); });
    connect(removeButton, &QPushButton::clicked, this, [this] { removeSelectedScore(); });
    connect(scoreTable_, &QTableWidget::cellDoubleClicked, this, [this](int, int) {
        editSelectedScore();
    });

    try {
        for (const auto& c : myCourses()) {
            scoreCourseCombo_->addItem(
                QString::fromUtf8(u8"%1 - %2")
                    .arg(QString::fromStdString(c.getCourseId()))
                    .arg(QString::fromStdString(c.getCourseName())),
                QString::fromStdString(c.getCourseId()));
        }
    } catch (...) {}

    return page;
}

void TeacherWindow::refreshMyScores() {
    const QString courseId = scoreCourseCombo_->currentData().toString();
    if (courseId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"未选择课程"), QString::fromUtf8(u8"请先选择一门课程。"));
        return;
    }

    try {
        const auto scores = appContext_.scoreService.findByCourse(session_, courseId.toStdString());
        scoreTable_->setRowCount(static_cast<int>(scores.size()));

        int row = 0;
        for (const auto& s : scores) {
            scoreTable_->setItem(row, 0, createReadOnlyItem(QString::fromStdString(s.getStudentId())));
            scoreTable_->setItem(row, 1, createReadOnlyItem(QString::fromStdString(s.getCourseId())));
            scoreTable_->setItem(row, 2, createReadOnlyItem(QString::fromStdString(s.getSemester())));
            scoreTable_->setItem(row, 3, createReadOnlyItem(QString::number(s.getUsualScore(), 'f', 1)));
            scoreTable_->setItem(row, 4, createReadOnlyItem(QString::number(s.getFinalScore(), 'f', 1)));
            scoreTable_->setItem(row, 5, createReadOnlyItem(QString::number(s.getTotalScore(), 'f', 1)));
            ++row;
        }

        statusBar()->showMessage(
            QString::fromUtf8(u8"课程 %1 成绩已加载，共 %2 条。").arg(courseId).arg(scores.size()), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"加载成绩失败"), e);
    }
}

void TeacherWindow::createScore() {
    const QString courseId = scoreCourseCombo_->currentData().toString();
    if (courseId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"未选择课程"), QString::fromUtf8(u8"请先选择一门课程再录入成绩。"));
        return;
    }

    ScoreEditDialog dialog(this);
    dialog.setCourseIdLocked(courseId.toStdString());
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    try {
        const Score score = dialog.score();
        appContext_.scoreService.upsert(session_, score);
        refreshMyScores();
        statusBar()->showMessage(QString::fromUtf8(u8"成绩已写入。"), 4000);
        QMessageBox::information(this, QString::fromUtf8(u8"保存成功"), QString::fromUtf8(u8"成绩记录已写入或更新。"));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"保存成绩失败"), e);
    }
}

void TeacherWindow::editSelectedScore() {
    QString studentId, courseId, semester;
    if (!selectedScoreKey(studentId, courseId, semester)) {
        QMessageBox::information(this, QString::fromUtf8(u8"未选择成绩"), QString::fromUtf8(u8"请先在表格中选择一条成绩记录。"));
        return;
    }

    const int row = scoreTable_->currentRow();
    const Score current(
        studentId.toStdString(),
        courseId.toStdString(),
        semester.toStdString(),
        scoreTable_->item(row, 3)->text().toDouble(),
        scoreTable_->item(row, 4)->text().toDouble(),
        scoreTable_->item(row, 5)->text().toDouble());

    ScoreEditDialog dialog(current, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    try {
        const Score updated = dialog.score();
        appContext_.scoreService.upsert(session_, updated);
        refreshMyScores();
        statusBar()->showMessage(QString::fromUtf8(u8"成绩已更新。"), 4000);
        QMessageBox::information(this, QString::fromUtf8(u8"更新成功"), QString::fromUtf8(u8"成绩记录已更新。"));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"更新成绩失败"), e);
    }
}

void TeacherWindow::removeSelectedScore() {
    QString studentId, courseId, semester;
    if (!selectedScoreKey(studentId, courseId, semester)) {
        QMessageBox::information(this, QString::fromUtf8(u8"未选择成绩"), QString::fromUtf8(u8"请先在表格中选择要删除的成绩记录。"));
        return;
    }

    const auto confirm = QMessageBox::warning(
        this,
        QString::fromUtf8(u8"确认删除"),
        QString::fromUtf8(u8"将删除成绩 %1 / %2 / %3，是否继续？").arg(studentId).arg(courseId).arg(semester),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (confirm != QMessageBox::Yes) {
        return;
    }

    try {
        appContext_.scoreService.remove(session_, studentId.toStdString(), courseId.toStdString(), semester.toStdString());
        refreshMyScores();
        statusBar()->showMessage(QString::fromUtf8(u8"成绩已删除。"), 4000);
        QMessageBox::information(this, QString::fromUtf8(u8"删除成功"), QString::fromUtf8(u8"成绩记录已删除。"));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"删除成绩失败"), e);
    }
}

bool TeacherWindow::selectedScoreKey(QString& studentId, QString& courseId, QString& semester) const {
    const int row = scoreTable_ ? scoreTable_->currentRow() : -1;
    if (row < 0) return false;

    const auto* s = scoreTable_->item(row, 0);
    const auto* c = scoreTable_->item(row, 1);
    const auto* sem = scoreTable_->item(row, 2);
    if (!s || !c || !sem) return false;

    studentId = s->text();
    courseId = c->text();
    semester = sem->text();
    return true;
}

// ---------------------------------------------------------------------------
// My stats page
// ---------------------------------------------------------------------------

QWidget* TeacherWindow::createMyStatsPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>我的课程统计</b>"), page);
    rootLayout->addWidget(titleLabel);

    auto* statsLayout = new QHBoxLayout();
    statsLayout->addWidget(new QLabel(QString::fromUtf8(u8"选择课程"), page));
    statsCourseCombo_ = new QComboBox(page);
    statsLayout->addWidget(statsCourseCombo_, 1);
    auto* courseStatsButton = new QPushButton(QString::fromUtf8(u8"课程统计"), page);
    auto* courseRankButton = new QPushButton(QString::fromUtf8(u8"课程排名"), page);
    statsLayout->addWidget(courseStatsButton);
    statsLayout->addWidget(courseRankButton);
    rootLayout->addLayout(statsLayout);

    rankingTable_ = new QTableWidget(page);
    rankingTable_->setColumnCount(3);
    rankingTable_->setHorizontalHeaderLabels({
        QString::fromUtf8(u8"学号"),
        QString::fromUtf8(u8"姓名"),
        QString::fromUtf8(u8"总评")
    });
    configureTable(rankingTable_);
    rootLayout->addWidget(rankingTable_, 1);

    connect(courseStatsButton, &QPushButton::clicked, this, [this] { queryCourseStats(); });
    connect(courseRankButton, &QPushButton::clicked, this, [this] { queryCourseRanking(); });

    try {
        for (const auto& c : myCourses()) {
            statsCourseCombo_->addItem(
                QString::fromUtf8(u8"%1 - %2")
                    .arg(QString::fromStdString(c.getCourseId()))
                    .arg(QString::fromStdString(c.getCourseName())),
                QString::fromStdString(c.getCourseId()));
        }
    } catch (...) {}

    return page;
}

void TeacherWindow::queryCourseStats() {
    const QString courseId = statsCourseCombo_->currentData().toString();
    if (courseId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"未选择课程"), QString::fromUtf8(u8"请先选择一门课程。"));
        return;
    }

    try {
        const CourseStats stats = appContext_.statsService.computeCourseStats(session_, courseId.toStdString());
        const QString detail = QString::fromUtf8(
            u8"课程：%1（%2）\n"
            u8"选课人数：%3\n"
            u8"平均分：%4\n"
            u8"最高分：%5\n"
            u8"最低分：%6\n"
            u8"及格率：%7%\n"
            u8"优秀率：%8%")
                .arg(QString::fromStdString(stats.courseId))
                .arg(QString::fromStdString(stats.courseName))
                .arg(stats.count)
                .arg(stats.avg, 0, 'f', 2)
                .arg(stats.max, 0, 'f', 1)
                .arg(stats.min, 0, 'f', 1)
                .arg(stats.passRate * 100.0, 0, 'f', 1)
                .arg(stats.excellentRate * 100.0, 0, 'f', 1);
        QMessageBox::information(this, QString::fromUtf8(u8"课程统计"), detail);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"查询课程统计失败"), e);
    }
}

void TeacherWindow::queryCourseRanking() {
    const QString courseId = statsCourseCombo_->currentData().toString();
    if (courseId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"未选择课程"), QString::fromUtf8(u8"请先选择一门课程。"));
        return;
    }

    try {
        const auto ranking = appContext_.statsService.rankByCourse(session_, courseId.toStdString());
        rankingTable_->setRowCount(static_cast<int>(ranking.size()));

        int row = 0;
        for (const auto& entry : ranking) {
            rankingTable_->setItem(row, 0, createReadOnlyItem(QString::fromStdString(entry.studentId)));
            rankingTable_->setItem(row, 1, createReadOnlyItem(QString::fromStdString(entry.studentName)));
            rankingTable_->setItem(row, 2, createReadOnlyItem(QString::number(entry.totalScore, 'f', 1)));
            ++row;
        }

        statusBar()->showMessage(
            QString::fromUtf8(u8"课程 %1 排名已加载，共 %2 人。").arg(courseId).arg(ranking.size()), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"查询课程排名失败"), e);
    }
}

// ---------------------------------------------------------------------------
// Account page
// ---------------------------------------------------------------------------

QWidget* TeacherWindow::createAccountPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>账户</b>"), page);
    auto* infoLabel = new QLabel(
        QString::fromUtf8(u8"当前用户：%1（角色：教师）")
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

void TeacherWindow::showServiceError(const QString& title, const std::exception& e) {
    QMessageBox::critical(this, title, QString::fromLocal8Bit(e.what()));
}

} // namespace EduSys
