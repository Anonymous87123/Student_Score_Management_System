#include "EduSys/gui/StudentWindow.hpp"

#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <QAbstractItemView>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "EduSys/app/AppContext.hpp"
#include "EduSys/gui/BackgroundHostWidget.hpp"
#include "EduSys/gui/BackgroundSettings.hpp"
#include "EduSys/gui/ChangePasswordDialog.hpp"
#include "EduSys/gui/ScoreBarChartWidget.hpp"
#include "EduSys/model/Course.hpp"
#include "EduSys/model/Score.hpp"
#include "EduSys/model/Student.hpp"
#include "EduSys/service/StatsService.hpp"

namespace {

constexpr const char* kStudentBackgroundRoleKey = "student";

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
        QScrollArea {
            background-color: rgba(248, 251, 255, 230);
            border: 1px solid #d6e0ee;
        }
        QStatusBar {
            background-color: #f8fbff;
            color: #475569;
        }
    )"));
}

} // namespace

namespace EduSys {

StudentWindow::StudentWindow(AppContext& appContext, Session session, QWidget* parent)
    : QMainWindow(parent)
    , appContext_(appContext)
    , session_(std::move(session)) {
    setWindowTitle(QString::fromUtf8(u8"EduSys 学生端"));
    resize(900, 640);
    applyLightTheme(this);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(createProfilePage(), QString::fromUtf8(u8"我的资料"));
    tabs->addTab(createMyCoursesPage(), QString::fromUtf8(u8"我的课程"));
    tabs->addTab(createMyScoresPage(), QString::fromUtf8(u8"我的成绩"));
    tabs->addTab(createScoreChartPage(), QString::fromUtf8(u8"成绩图表"));
    tabs->addTab(createMyGpaPage(), QString::fromUtf8(u8"我的 GPA"));
    tabs->addTab(createAccountPage(), QString::fromUtf8(u8"账户"));
    tabs->addTab(createBackgroundSettingsPage(), QString::fromUtf8(u8"背景设置"));

    setCentralWidget(tabs);
    statusBar()->showMessage(
        QString::fromUtf8(u8"当前学生：%1")
            .arg(QString::fromStdString(session_.getUsername())));
}

// ---------------------------------------------------------------------------
// Profile page
// ---------------------------------------------------------------------------

QWidget* StudentWindow::createProfilePage() {
    auto* page = createBackgroundPage();
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
// My courses page
// ---------------------------------------------------------------------------

QWidget* StudentWindow::createMyCoursesPage() {
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

void StudentWindow::refreshMyCourses() {
    try {
        const QString semesterFilter = courseSemesterFilter_ ? courseSemesterFilter_->text().trimmed() : QString();
        const auto scores = semesterFilter.isEmpty()
            ? appContext_.scoreService.findByStudent(session_, session_.getOwnerId())
            : appContext_.scoreService.findByStudentAndSemester(
                session_, session_.getOwnerId(), semesterFilter.toStdString());
        const auto courses = appContext_.courseService.listAll(session_);

        std::unordered_map<std::string, Course> courseMap;
        for (const auto& course : courses) {
            courseMap[course.getCourseId()] = course;
        }

        std::unordered_set<std::string> seenKeys;
        std::vector<std::vector<QString>> rows;
        for (const auto& score : scores) {
            const std::string key = score.getCourseId() + "\n" + score.getSemester();
            if (!seenKeys.insert(key).second) {
                continue;
            }
            const auto it = courseMap.find(score.getCourseId());
            const QString courseName = it == courseMap.end()
                ? QStringLiteral("-")
                : QString::fromStdString(it->second.getCourseName());
            const QString credit = it == courseMap.end()
                ? QStringLiteral("-")
                : QString::number(it->second.getCredit(), 'f', 1);
            rows.push_back({
                QString::fromStdString(score.getCourseId()),
                courseName,
                credit,
                QString::fromStdString(score.getSemester())
            });
        }

        courseTable_->setRowCount(static_cast<int>(rows.size()));
        int row = 0;
        for (const auto& values : rows) {
            for (int col = 0; col < static_cast<int>(values.size()); ++col) {
                courseTable_->setItem(row, col, createReadOnlyItem(values[static_cast<std::size_t>(col)]));
            }
            ++row;
        }

        statusBar()->showMessage(
            QString::fromUtf8(u8"我的课程已查询，共 %1 门。").arg(rows.size()), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"刷新我的课程失败"), e);
    }
}

// ---------------------------------------------------------------------------
// My scores page
// ---------------------------------------------------------------------------

QWidget* StudentWindow::createMyScoresPage() {
    auto* page = createBackgroundPage();
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>我的成绩</b>"), page);
    rootLayout->addWidget(titleLabel);

    auto* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"课程号"), page));
    scoreCourseFilter_ = new QLineEdit(page);
    scoreCourseFilter_->setPlaceholderText(QString::fromUtf8(u8"留空=全部"));
    filterLayout->addWidget(scoreCourseFilter_, 1);
    filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"学期"), page));
    scoreSemesterFilter_ = new QLineEdit(page);
    scoreSemesterFilter_->setPlaceholderText(QString::fromUtf8(u8"留空=全部"));
    filterLayout->addWidget(scoreSemesterFilter_, 1);
    auto* refreshButton = new QPushButton(QString::fromUtf8(u8"查询成绩"), page);
    filterLayout->addWidget(refreshButton);
    rootLayout->addLayout(filterLayout);

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
        const QString courseId = scoreCourseFilter_ ? scoreCourseFilter_->text().trimmed() : QString();
        const QString semester = scoreSemesterFilter_ ? scoreSemesterFilter_->text().trimmed() : QString();
        scoreRows_ = appContext_.scoreService.query(
            session_,
            session_.getOwnerId(),
            courseId.toStdString(),
            {},
            semester.toStdString());
        scoreTable_->setRowCount(static_cast<int>(scoreRows_.size()));

        int row = 0;
        for (const auto& s : scoreRows_) {
            scoreTable_->setItem(row, 0, createReadOnlyItem(QString::fromStdString(s.getCourseId())));
            scoreTable_->setItem(row, 1, createReadOnlyItem(QString::fromStdString(s.getSemester())));
            scoreTable_->setItem(row, 2, createReadOnlyItem(QString::number(s.getUsualScore(), 'f', 1)));
            scoreTable_->setItem(row, 3, createReadOnlyItem(QString::number(s.getFinalScore(), 'f', 1)));
            scoreTable_->setItem(row, 4, createReadOnlyItem(QString::number(s.getTotalScore(), 'f', 1)));
            ++row;
        }

        statusBar()->showMessage(
            QString::fromUtf8(u8"我的成绩已查询，共 %1 条。").arg(scoreRows_.size()), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"刷新成绩失败"), e);
    }
}

// ---------------------------------------------------------------------------
// Score chart page
// ---------------------------------------------------------------------------

QWidget* StudentWindow::createScoreChartPage() {
    auto* page = createBackgroundPage();
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>成绩图表</b>"), page);
    rootLayout->addWidget(titleLabel);

    auto* noteLabel = new QLabel(
        QString::fromUtf8(u8"按课程和学期展示当前学生的总评分数。绿色表示 90 分及以上，蓝色表示 80-89 分，黄色表示 60-79 分，红色表示未及格。"),
        page);
    noteLabel->setWordWrap(true);
    rootLayout->addWidget(noteLabel);

    auto* refreshButton = new QPushButton(QString::fromUtf8(u8"刷新图表"), page);
    refreshButton->setMaximumWidth(160);
    rootLayout->addWidget(refreshButton);

    auto* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scoreChart_ = new ScoreBarChartWidget(scrollArea);
    scrollArea->setWidget(scoreChart_);
    rootLayout->addWidget(scrollArea, 1);

    connect(refreshButton, &QPushButton::clicked, this, [this] { refreshScoreChart(); });

    refreshScoreChart();
    return page;
}

void StudentWindow::refreshScoreChart() {
    try {
        if (scoreRows_.empty()) {
            scoreRows_ = appContext_.scoreService.findByStudent(session_, session_.getOwnerId());
        }
        scoreChart_->setScores(scoreRows_);
        statusBar()->showMessage(
            QString::fromUtf8(u8"成绩图表已刷新，共 %1 条。").arg(scoreRows_.size()), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"刷新成绩图表失败"), e);
    }
}

// ---------------------------------------------------------------------------
// My GPA page
// ---------------------------------------------------------------------------

QWidget* StudentWindow::createMyGpaPage() {
    auto* page = createBackgroundPage();
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>我的 GPA</b>"), page);
    rootLayout->addWidget(titleLabel);

    gpaLabel_ = new QLabel(page);
    gpaLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    gpaLabel_->setStyleSheet(QString::fromLatin1(
        "color: #0f172a; font-size: 15px; line-height: 1.6;"));
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
    auto* page = createBackgroundPage();
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

QWidget* StudentWindow::createBackgroundSettingsPage() {
    auto* page = createBackgroundPage();
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>背景设置</b>"), page);
    auto* introLabel = new QLabel(
        QString::fromUtf8(u8"可以选择一张 PNG 图片作为当前学生端背景，也可以调整透明度。表格和图表仍保留浅色底，避免影响阅读。"),
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
        GuiBackground::loadOpacity(QString::fromLatin1(kStudentBackgroundRoleKey)) * 100.0));
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

BackgroundHostWidget* StudentWindow::createBackgroundPage() {
    auto* page = new BackgroundHostWidget(this);
    page->setObjectName(QStringLiteral("RoleBackgroundPage"));
    GuiBackground::applyBackgroundToPage(page, QString::fromLatin1(kStudentBackgroundRoleKey));
    backgroundPages_.push_back(page);
    return page;
}

void StudentWindow::refreshBackgroundPages() {
    GuiBackground::applyBackgroundToPages(backgroundPages_, QString::fromLatin1(kStudentBackgroundRoleKey));
}

void StudentWindow::chooseBackgroundImage() {
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
            QString::fromLatin1(kStudentBackgroundRoleKey), sourcePath, &errorMessage)) {
        QMessageBox::warning(this, QString::fromUtf8(u8"背景设置失败"), errorMessage);
        return;
    }

    refreshBackgroundPages();
    QMessageBox::information(this, QString::fromUtf8(u8"背景已更新"), QString::fromUtf8(u8"背景图片已应用。"));
}

void StudentWindow::clearBackgroundImage() {
    QString errorMessage;
    if (!GuiBackground::clearBackgroundImage(QString::fromLatin1(kStudentBackgroundRoleKey), &errorMessage)) {
        QMessageBox::warning(this, QString::fromUtf8(u8"背景设置失败"), errorMessage);
        return;
    }

    refreshBackgroundPages();
    QMessageBox::information(this, QString::fromUtf8(u8"背景已清除"), QString::fromUtf8(u8"已恢复浅色默认背景。"));
}

void StudentWindow::updateBackgroundOpacity(int value) {
    GuiBackground::saveOpacity(QString::fromLatin1(kStudentBackgroundRoleKey), value / 100.0);
    refreshBackgroundPages();
}

void StudentWindow::showServiceError(const QString& title, const std::exception& e) {
    QMessageBox::critical(this, title, QString::fromLocal8Bit(e.what()));
}

} // namespace EduSys

