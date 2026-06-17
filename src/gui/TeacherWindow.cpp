#include "EduSys/gui/TeacherWindow.hpp"

#include <algorithm>
#include <utility>
#include <unordered_map>
#include <vector>

#include <QAbstractItemView>
#include <QCollator>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QLocale>
#include <QPushButton>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QSlider>

#include "EduSys/app/AppContext.hpp"
#include "EduSys/gui/BackgroundHostWidget.hpp"
#include "EduSys/gui/BackgroundSettings.hpp"
#include "EduSys/gui/ChangePasswordDialog.hpp"
#include "EduSys/gui/ScoreEditDialog.hpp"
#include "EduSys/model/Score.hpp"
#include "EduSys/service/StatsService.hpp"

namespace {

constexpr const char* kTeacherBackgroundRoleKey = "teacher";

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

std::string lookupStudentName(const std::unordered_map<std::string, std::string>& names,
                              const std::string& studentId) {
    const auto it = names.find(studentId);
    return it == names.end() ? std::string{} : it->second;
}

void sortScoresByStudentId(std::vector<EduSys::Score>& scores) {
    QCollator collator(QLocale::c());
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);

    std::sort(scores.begin(), scores.end(), [&](const EduSys::Score& lhs, const EduSys::Score& rhs) {
        const int idCompare = collator.compare(
            QString::fromStdString(lhs.getStudentId()),
            QString::fromStdString(rhs.getStudentId()));
        if (idCompare != 0) {
            return idCompare < 0;
        }
        return QString::fromStdString(lhs.getSemester()) < QString::fromStdString(rhs.getSemester());
    });
}

void sortScoresByStudentName(std::vector<EduSys::Score>& scores,
                             const std::unordered_map<std::string, std::string>& names) {
    QCollator collator(QLocale(QLocale::Chinese, QLocale::China));
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);

    std::sort(scores.begin(), scores.end(), [&](const EduSys::Score& lhs, const EduSys::Score& rhs) {
        const int nameCompare = collator.compare(
            QString::fromStdString(lookupStudentName(names, lhs.getStudentId())),
            QString::fromStdString(lookupStudentName(names, rhs.getStudentId())));
        if (nameCompare != 0) {
            return nameCompare < 0;
        }
        return collator.compare(
            QString::fromStdString(lhs.getStudentId()),
            QString::fromStdString(rhs.getStudentId())) < 0;
    });
}

void sortScoresByTotalDesc(std::vector<EduSys::Score>& scores) {
    QCollator collator(QLocale::c());
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);

    std::sort(scores.begin(), scores.end(), [&](const EduSys::Score& lhs, const EduSys::Score& rhs) {
        if (lhs.getTotalScore() != rhs.getTotalScore()) {
            return lhs.getTotalScore() > rhs.getTotalScore();
        }
        return collator.compare(
            QString::fromStdString(lhs.getStudentId()),
            QString::fromStdString(rhs.getStudentId())) < 0;
    });
}

void applyLightTheme(QWidget* root) {
    root->setAttribute(Qt::WA_StyledBackground, true);
    root->setStyleSheet(QString::fromLatin1(R"(
        QMainWindow {
            background-color: #f5f8fc;
        }
        QWidget#RoleBackgroundPage {
            background-color: transparent;
        }
        QWidget {
            color: #1f2937;
            font-size: 13px;
        }
        QTabWidget::pane {
            background-color: rgba(248, 251, 255, 210);
            border: 1px solid #d6e0ee;
            top: -1px;
        }
        QTabBar::tab {
            background-color: #eaf0f7;
            color: #334155;
            border: 1px solid #d6e0ee;
            border-bottom: none;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            padding: 8px 14px;
            min-width: 96px;
        }
        QTabBar::tab:selected {
            background-color: #ffffff;
            color: #0f172a;
        }
        QTabBar::tab:hover {
            background-color: #f4f8fc;
        }
        QTableWidget {
            background-color: #ffffff;
            alternate-background-color: #f8fbff;
            gridline-color: #dbe4ee;
            selection-background-color: #cfe3ff;
            selection-color: #0f172a;
        }
        QHeaderView::section {
            background-color: #edf3f8;
            color: #334155;
            border: 1px solid #d6e0ee;
            padding: 6px 8px;
        }
        QLineEdit, QComboBox {
            background-color: #ffffff;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            padding: 6px 8px;
            selection-background-color: #bfdbfe;
        }
        QPushButton {
            background-color: #ffffff;
            color: #0f172a;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            padding: 7px 14px;
        }
        QPushButton:hover {
            background-color: #f8fbff;
            border-color: #7fb0e9;
        }
        QPushButton:pressed {
            background-color: #eaf2ff;
        }
        QPushButton:disabled {
            background-color: #edf2f7;
            color: #94a3b8;
            border-color: #dbe4ee;
        }
        QLabel {
            color: #1f2937;
        }
        QStatusBar {
            background-color: #f8fbff;
            color: #475569;
        }
    )"));
}

} // namespace

namespace EduSys {

TeacherWindow::TeacherWindow(AppContext& appContext, Session session, QWidget* parent)
    : QMainWindow(parent)
    , appContext_(appContext)
    , session_(std::move(session)) {
    setWindowTitle(QString::fromUtf8(u8"EduSys 教师端"));
    resize(1000, 680);
    applyLightTheme(this);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(createMyCoursesPage(), QString::fromUtf8(u8"我的课程"));
    tabs->addTab(createMyScoresPage(), QString::fromUtf8(u8"我的课程成绩"));
    tabs->addTab(createMyStatsPage(), QString::fromUtf8(u8"我的课程统计"));
    tabs->addTab(createAccountPage(), QString::fromUtf8(u8"账户"));
    tabs->addTab(createBackgroundSettingsPage(), QString::fromUtf8(u8"背景设置"));

    setCentralWidget(tabs);
    statusBar()->showMessage(
        QString::fromUtf8(u8"当前教师：%1")
            .arg(QString::fromStdString(session_.getUsername())));
}

std::vector<Course> TeacherWindow::myCourses() {
    return appContext_.courseService.listAll(session_);
}

// ---------------------------------------------------------------------------
// My courses page
// ---------------------------------------------------------------------------

QWidget* TeacherWindow::createMyCoursesPage() {
    auto* page = createBackgroundPage();
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>我的课程</b>"), page);
    rootLayout->addWidget(titleLabel);

    auto* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"学期"), page));
    courseSemesterFilter_ = new QLineEdit(page);
    courseSemesterFilter_->setPlaceholderText(QString::fromUtf8(u8"留空=全部，如 2025-2026-1"));
    filterLayout->addWidget(courseSemesterFilter_, 1);
    auto* refreshButton = new QPushButton(QString::fromUtf8(u8"查询课程"), page);
    filterLayout->addWidget(refreshButton);
    rootLayout->addLayout(filterLayout);

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
        const QString semester = courseSemesterFilter_ ? courseSemesterFilter_->text().trimmed() : QString();
        const auto courses = semester.isEmpty()
            ? myCourses()
            : appContext_.courseService.listByTeacherAndSemester(
                session_, session_.getOwnerId(), semester.toStdString());
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
            QString::fromUtf8(u8"我的课程已查询，共 %1 门。").arg(courses.size()), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"刷新我的课程失败"), e);
    }
}

// ---------------------------------------------------------------------------
// My scores page
// ---------------------------------------------------------------------------

QWidget* TeacherWindow::createMyScoresPage() {
    auto* page = createBackgroundPage();
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>我的课程成绩</b>"), page);
    rootLayout->addWidget(titleLabel);

    auto* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"选择课程"), page));
    scoreCourseCombo_ = new QComboBox(page);
    scoreCourseCombo_->addItem(QString::fromUtf8(u8"全部课程"), QString());
    filterLayout->addWidget(scoreCourseCombo_, 1);
    filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"学生"), page));
    scoreStudentFilter_ = new QLineEdit(page);
    scoreStudentFilter_->setPlaceholderText(QString::fromUtf8(u8"留空=全部"));
    scoreStudentFilter_->setMaximumWidth(140);
    filterLayout->addWidget(scoreStudentFilter_);
    filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"班级"), page));
    scoreClassFilter_ = new QLineEdit(page);
    scoreClassFilter_->setPlaceholderText(QString::fromUtf8(u8"留空=全部"));
    scoreClassFilter_->setMaximumWidth(140);
    filterLayout->addWidget(scoreClassFilter_);
    filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"学期"), page));
    scoreSemesterFilter_ = new QLineEdit(page);
    scoreSemesterFilter_->setPlaceholderText(QString::fromUtf8(u8"留空=全部"));
    scoreSemesterFilter_->setMaximumWidth(140);
    filterLayout->addWidget(scoreSemesterFilter_);
    auto* loadButton = new QPushButton(QString::fromUtf8(u8"加载成绩"), page);
    filterLayout->addWidget(loadButton);
    rootLayout->addLayout(filterLayout);

    auto* actionLayout = new QHBoxLayout();
    auto* createButton = new QPushButton(QString::fromUtf8(u8"录入成绩"), page);
    auto* editButton = new QPushButton(QString::fromUtf8(u8"编辑选中"), page);
    auto* removeButton = new QPushButton(QString::fromUtf8(u8"删除选中"), page);
    auto* sortByIdButton = new QPushButton(QString::fromUtf8(u8"按学号排序"), page);
    auto* sortByNameButton = new QPushButton(QString::fromUtf8(u8"按姓氏首字母排序"), page);
    auto* sortByTotalButton = new QPushButton(QString::fromUtf8(u8"按成绩排序"), page);
    actionLayout->addWidget(createButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addSpacing(18);
    actionLayout->addWidget(new QLabel(QString::fromUtf8(u8"排序"), page));
    actionLayout->addWidget(sortByIdButton);
    actionLayout->addWidget(sortByNameButton);
    actionLayout->addWidget(sortByTotalButton);
    actionLayout->addStretch();
    rootLayout->addLayout(actionLayout);

    scoreTable_ = new QTableWidget(page);
    scoreTable_->setColumnCount(7);
    scoreTable_->setHorizontalHeaderLabels({
        QString::fromUtf8(u8"学号"),
        QString::fromUtf8(u8"姓名"),
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
    connect(sortByIdButton, &QPushButton::clicked, this, [this] { showScoresSortedByStudentId(); });
    connect(sortByNameButton, &QPushButton::clicked, this, [this] { showScoresSortedByStudentName(); });
    connect(sortByTotalButton, &QPushButton::clicked, this, [this] { showScoresSortedByTotalDesc(); });
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
    const QString studentId = scoreStudentFilter_ ? scoreStudentFilter_->text().trimmed() : QString();
    const QString className = scoreClassFilter_ ? scoreClassFilter_->text().trimmed() : QString();
    const QString semester = scoreSemesterFilter_ ? scoreSemesterFilter_->text().trimmed() : QString();

    try {
        scoreRows_ = appContext_.scoreService.query(
            session_,
            studentId.toStdString(),
            courseId.toStdString(),
            className.toStdString(),
            semester.toStdString());
        scoreStudentNames_.clear();
        for (const auto& student : appContext_.studentService.listAll(session_)) {
            scoreStudentNames_[student.getId()] = student.getName();
        }

        sortScoresByStudentId(scoreRows_);
        populateScoreTable();

        statusBar()->showMessage(
            QString::fromUtf8(u8"课程 %1 成绩已加载，共 %2 条。").arg(courseId).arg(scoreRows_.size()), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"加载成绩失败"), e);
    }
}

void TeacherWindow::showScoresSortedByStudentId() {
    sortScoresByStudentId(scoreRows_);
    populateScoreTable();
    statusBar()->showMessage(QString::fromUtf8(u8"课程成绩已按学号排序。"), 4000);
}

void TeacherWindow::showScoresSortedByStudentName() {
    sortScoresByStudentName(scoreRows_, scoreStudentNames_);
    populateScoreTable();
    statusBar()->showMessage(QString::fromUtf8(u8"课程成绩已按姓氏首字母排序。"), 4000);
}

void TeacherWindow::showScoresSortedByTotalDesc() {
    sortScoresByTotalDesc(scoreRows_);
    populateScoreTable();
    statusBar()->showMessage(QString::fromUtf8(u8"课程成绩已按总评从高到低排序。"), 4000);
}

QString TeacherWindow::studentNameFor(const std::string& studentId) const {
    const auto it = scoreStudentNames_.find(studentId);
    if (it == scoreStudentNames_.end()) {
        return QStringLiteral("-");
    }
    return QString::fromStdString(it->second);
}

void TeacherWindow::populateScoreTable() {
    scoreTable_->setRowCount(static_cast<int>(scoreRows_.size()));

    int row = 0;
    for (const auto& s : scoreRows_) {
        scoreTable_->setItem(row, 0, createReadOnlyItem(QString::fromStdString(s.getStudentId())));
        scoreTable_->setItem(row, 1, createReadOnlyItem(studentNameFor(s.getStudentId())));
        scoreTable_->setItem(row, 2, createReadOnlyItem(QString::fromStdString(s.getCourseId())));
        scoreTable_->setItem(row, 3, createReadOnlyItem(QString::fromStdString(s.getSemester())));
        scoreTable_->setItem(row, 4, createReadOnlyItem(QString::number(s.getUsualScore(), 'f', 1)));
        scoreTable_->setItem(row, 5, createReadOnlyItem(QString::number(s.getFinalScore(), 'f', 1)));
        scoreTable_->setItem(row, 6, createReadOnlyItem(QString::number(s.getTotalScore(), 'f', 1)));
        ++row;
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
        scoreTable_->item(row, 4)->text().toDouble(),
        scoreTable_->item(row, 5)->text().toDouble(),
        scoreTable_->item(row, 6)->text().toDouble());

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
    const auto* c = scoreTable_->item(row, 2);
    const auto* sem = scoreTable_->item(row, 3);
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
    auto* page = createBackgroundPage();
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
    auto* page = createBackgroundPage();
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

QWidget* TeacherWindow::createBackgroundSettingsPage() {
    auto* page = createBackgroundPage();
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>背景设置</b>"), page);
    auto* introLabel = new QLabel(
        QString::fromUtf8(u8"可以选择一张 PNG 图片作为当前教师端背景，也可以调整透明度，避免影响表格和文字阅读。"),
        page);
    introLabel->setWordWrap(true);
    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(introLabel);
    rootLayout->addSpacing(12);

    auto* actionLayout = new QHBoxLayout();
    auto* chooseButton = new QPushButton(QString::fromUtf8(u8"选择 PNG 背景"), page);
    auto* clearButton = new QPushButton(QString::fromUtf8(u8"清除背景"), page);
    actionLayout->addWidget(chooseButton);
    actionLayout->addWidget(clearButton);
    actionLayout->addStretch();
    rootLayout->addLayout(actionLayout);

    auto* opacityLayout = new QHBoxLayout();
    opacityLayout->addWidget(new QLabel(QString::fromUtf8(u8"背景透明度"), page));
    auto* opacitySlider = new QSlider(Qt::Horizontal, page);
    opacitySlider->setRange(5, 45);
    opacitySlider->setValue(static_cast<int>(
        GuiBackground::loadOpacity(QString::fromLatin1(kTeacherBackgroundRoleKey)) * 100.0));
    opacityLayout->addWidget(opacitySlider, 1);
    rootLayout->addLayout(opacityLayout);
    rootLayout->addStretch();

    connect(chooseButton, &QPushButton::clicked, this, [this] {
        chooseBackgroundImage();
    });
    connect(clearButton, &QPushButton::clicked, this, [this] {
        clearBackgroundImage();
    });
    connect(opacitySlider, &QSlider::valueChanged, this, [this](int value) {
        updateBackgroundOpacity(value);
    });

    return page;
}

BackgroundHostWidget* TeacherWindow::createBackgroundPage() {
    auto* page = new BackgroundHostWidget(this);
    page->setObjectName(QStringLiteral("RoleBackgroundPage"));
    GuiBackground::applyBackgroundToPage(page, QString::fromLatin1(kTeacherBackgroundRoleKey));
    backgroundPages_.push_back(page);
    return page;
}

void TeacherWindow::refreshBackgroundPages() {
    GuiBackground::applyBackgroundToPages(backgroundPages_, QString::fromLatin1(kTeacherBackgroundRoleKey));
}

void TeacherWindow::chooseBackgroundImage() {
    const QString sourcePath = QFileDialog::getOpenFileName(
        this,
        QString::fromUtf8(u8"选择 PNG 背景"),
        QString(),
        QString::fromUtf8(u8"PNG 图片 (*.png)"));
    if (sourcePath.isEmpty()) {
        return;
    }

    QString errorMessage;
    if (!GuiBackground::replaceBackgroundImage(
            QString::fromLatin1(kTeacherBackgroundRoleKey), sourcePath, &errorMessage)) {
        QMessageBox::warning(this, QString::fromUtf8(u8"背景设置失败"), errorMessage);
        return;
    }

    refreshBackgroundPages();
    QMessageBox::information(this, QString::fromUtf8(u8"背景已更新"), QString::fromUtf8(u8"背景图片已应用。"));
}

void TeacherWindow::clearBackgroundImage() {
    QString errorMessage;
    if (!GuiBackground::clearBackgroundImage(QString::fromLatin1(kTeacherBackgroundRoleKey), &errorMessage)) {
        QMessageBox::warning(this, QString::fromUtf8(u8"背景设置失败"), errorMessage);
        return;
    }

    refreshBackgroundPages();
    QMessageBox::information(this, QString::fromUtf8(u8"背景已清除"), QString::fromUtf8(u8"已恢复浅色默认背景。"));
}

void TeacherWindow::updateBackgroundOpacity(int value) {
    GuiBackground::saveOpacity(QString::fromLatin1(kTeacherBackgroundRoleKey), value / 100.0);
    refreshBackgroundPages();
}

void TeacherWindow::showServiceError(const QString& title, const std::exception& e) {
    QMessageBox::critical(this, title, QString::fromLocal8Bit(e.what()));
}

} // namespace EduSys
