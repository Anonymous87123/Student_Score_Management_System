#include "EduSys/gui/AdminWindow.hpp"

#include <utility>
#include <vector>

#include <QAbstractItemView>
#include <QHeaderView>
#include <QHBoxLayout>
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
#include "EduSys/gui/CourseEditDialog.hpp"
#include "EduSys/gui/ScoreEditDialog.hpp"
#include "EduSys/gui/StudentEditDialog.hpp"
#include "EduSys/model/Course.hpp"
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

void populateStudentTable(QTableWidget* table, const std::vector<EduSys::Student>& students) {
    table->setRowCount(static_cast<int>(students.size()));

    int row = 0;
    for (const auto& student : students) {
        table->setItem(row, 0, createReadOnlyItem(QString::fromStdString(student.getId())));
        table->setItem(row, 1, createReadOnlyItem(QString::fromStdString(student.getName())));
        table->setItem(row, 2, createReadOnlyItem(QString::fromStdString(student.getMajor())));
        table->setItem(row, 3, createReadOnlyItem(QString::fromStdString(student.getClassName())));
        table->setItem(row, 4, createReadOnlyItem(QString::number(student.getEnrollYear())));
        table->setItem(row, 5, createReadOnlyItem(QString::fromStdString(student.getContact())));
        ++row;
    }
}

void populateCourseTable(QTableWidget* table, const std::vector<EduSys::Course>& courses) {
    table->setRowCount(static_cast<int>(courses.size()));

    int row = 0;
    for (const auto& course : courses) {
        table->setItem(row, 0, createReadOnlyItem(QString::fromStdString(course.getCourseId())));
        table->setItem(row, 1, createReadOnlyItem(QString::fromStdString(course.getCourseName())));
        table->setItem(row, 2, createReadOnlyItem(QString::number(course.getCredit(), 'f', 1)));
        table->setItem(row, 3, createReadOnlyItem(QString::fromStdString(course.getTeacherId())));
        table->setItem(row, 4, createReadOnlyItem(QString::fromStdString(course.getSemester())));
        ++row;
    }
}

void populateScoreTable(QTableWidget* table, const std::vector<EduSys::Score>& scores) {
    table->setRowCount(static_cast<int>(scores.size()));

    int row = 0;
    for (const auto& score : scores) {
        table->setItem(row, 0, createReadOnlyItem(QString::fromStdString(score.getStudentId())));
        table->setItem(row, 1, createReadOnlyItem(QString::fromStdString(score.getCourseId())));
        table->setItem(row, 2, createReadOnlyItem(QString::fromStdString(score.getSemester())));
        table->setItem(row, 3, createReadOnlyItem(QString::number(score.getUsualScore(), 'f', 1)));
        table->setItem(row, 4, createReadOnlyItem(QString::number(score.getFinalScore(), 'f', 1)));
        table->setItem(row, 5, createReadOnlyItem(QString::number(score.getTotalScore(), 'f', 1)));
        ++row;
    }
}

} // namespace

namespace EduSys {

AdminWindow::AdminWindow(AppContext& appContext, Session session, QWidget* parent)
    : QMainWindow(parent)
    , appContext_(appContext)
    , session_(std::move(session)) {
    setWindowTitle(QString::fromUtf8(u8"EduSys 管理员端"));
    resize(1180, 760);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(createStudentPage(), QString::fromUtf8(u8"学生管理"));
    tabs->addTab(createCoursePage(), QString::fromUtf8(u8"课程管理"));
    tabs->addTab(createScorePage(), QString::fromUtf8(u8"成绩管理"));
    tabs->addTab(createStatsPage(), QString::fromUtf8(u8"统计分析"));
    tabs->addTab(createReportPage(), QString::fromUtf8(u8"报告导出"));
    tabs->addTab(createAccountPage(), QString::fromUtf8(u8"账户"));

    setCentralWidget(tabs);
    statusBar()->showMessage(
        QString::fromUtf8(u8"当前用户：%1")
            .arg(QString::fromStdString(session_.getUsername())));
}

QWidget* AdminWindow::createStudentPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>学生管理</b>"), page);
    auto* introLabel = new QLabel(
        QString::fromUtf8(u8"可查看学生列表、按学号查询、新增、编辑和删除学生。"),
        page);
    introLabel->setWordWrap(true);

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(introLabel);

    auto* lookupLayout = new QHBoxLayout();
    lookupLayout->addWidget(new QLabel(QString::fromUtf8(u8"按学号查看"), page));
    studentLookupEdit_ = new QLineEdit(page);
    studentLookupEdit_->setPlaceholderText(QString::fromUtf8(u8"例如：S001"));
    lookupLayout->addWidget(studentLookupEdit_, 1);

    auto* viewButton = new QPushButton(QString::fromUtf8(u8"查看"), page);
    auto* refreshButton = new QPushButton(QString::fromUtf8(u8"刷新列表"), page);
    lookupLayout->addWidget(viewButton);
    lookupLayout->addWidget(refreshButton);
    rootLayout->addLayout(lookupLayout);

    auto* actionLayout = new QHBoxLayout();
    auto* createButton = new QPushButton(QString::fromUtf8(u8"新增学生"), page);
    auto* editButton = new QPushButton(QString::fromUtf8(u8"编辑选中"), page);
    auto* removeButton = new QPushButton(QString::fromUtf8(u8"删除选中"), page);
    actionLayout->addWidget(createButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    rootLayout->addLayout(actionLayout);

    studentTable_ = new QTableWidget(page);
    studentTable_->setColumnCount(6);
    studentTable_->setHorizontalHeaderLabels({
        QString::fromUtf8(u8"学号"),
        QString::fromUtf8(u8"姓名"),
        QString::fromUtf8(u8"专业"),
        QString::fromUtf8(u8"班级"),
        QString::fromUtf8(u8"入学年份"),
        QString::fromUtf8(u8"联系方式")
    });
    configureTable(studentTable_);
    rootLayout->addWidget(studentTable_, 1);

    connect(refreshButton, &QPushButton::clicked, this, [this] { refreshStudentTable(); });
    connect(viewButton, &QPushButton::clicked, this, [this] { showStudentById(); });
    connect(createButton, &QPushButton::clicked, this, [this] { createStudent(); });
    connect(editButton, &QPushButton::clicked, this, [this] { editSelectedStudent(); });
    connect(removeButton, &QPushButton::clicked, this, [this] { removeSelectedStudent(); });
    connect(studentLookupEdit_, &QLineEdit::returnPressed, this, [this] { showStudentById(); });
    connect(studentTable_, &QTableWidget::cellDoubleClicked, this, [this](int, int) {
        editSelectedStudent();
    });

    refreshStudentTable();
    return page;
}

void AdminWindow::refreshStudentTable() {
    try {
        const auto students = appContext_.studentService.listAll(session_);
        populateStudentTable(studentTable_, students);
        statusBar()->showMessage(
            QString::fromUtf8(u8"学生列表已刷新，共 %1 条。").arg(students.size()),
            4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"刷新学生列表失败"), e);
    }
}

void AdminWindow::showStudentById() {
    const QString id = studentLookupEdit_->text().trimmed();
    if (id.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"缺少学号"), QString::fromUtf8(u8"请先输入要查看的学号。"));
        studentLookupEdit_->setFocus();
        return;
    }

    try {
        const Student student = appContext_.studentService.findById(session_, id.toStdString());
        showStudentDetails(student);
        statusBar()->showMessage(QString::fromUtf8(u8"已查看学生：%1").arg(id), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"查询学生失败"), e);
    }
}

void AdminWindow::createStudent() {
    StudentEditDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    try {
        const Student student = dialog.student();
        appContext_.studentService.create(session_, student);
        refreshStudentTable();
        studentLookupEdit_->setText(QString::fromStdString(student.getId()));
        statusBar()->showMessage(
            QString::fromUtf8(u8"学生已新增：%1").arg(QString::fromStdString(student.getId())),
            4000);
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"新增成功"),
            QString::fromUtf8(u8"学生 %1 已创建。").arg(QString::fromStdString(student.getId())));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"新增学生失败"), e);
    }
}

void AdminWindow::editSelectedStudent() {
    const QString studentId = selectedStudentId();
    if (studentId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"未选择学生"), QString::fromUtf8(u8"请先在表格中选择一名学生。"));
        return;
    }

    try {
        const Student current = appContext_.studentService.findById(session_, studentId.toStdString());
        StudentEditDialog dialog(current, this);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        const Student updated = dialog.student();
        appContext_.studentService.update(session_, updated);
        refreshStudentTable();
        studentLookupEdit_->setText(studentId);
        statusBar()->showMessage(QString::fromUtf8(u8"学生已更新：%1").arg(studentId), 4000);
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"更新成功"),
            QString::fromUtf8(u8"学生 %1 已更新。").arg(studentId));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"编辑学生失败"), e);
    }
}

void AdminWindow::removeSelectedStudent() {
    const QString studentId = selectedStudentId();
    if (studentId.isEmpty()) {
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"未选择学生"),
            QString::fromUtf8(u8"请先在表格中选择要删除的学生。"));
        return;
    }

    const auto confirm = QMessageBox::warning(
        this,
        QString::fromUtf8(u8"确认级联删除"),
        QString::fromUtf8(u8"删除学生 %1 后，将同时删除该学生关联的成绩记录和学生账户。\n此操作不可撤销，是否继续？")
            .arg(studentId),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (confirm != QMessageBox::Yes) {
        return;
    }

    try {
        appContext_.studentService.remove(session_, studentId.toStdString());
        refreshStudentTable();
        studentLookupEdit_->clear();
        statusBar()->showMessage(QString::fromUtf8(u8"学生已删除：%1").arg(studentId), 4000);
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"删除成功"),
            QString::fromUtf8(u8"学生 %1 已级联删除。").arg(studentId));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"删除学生失败"), e);
    }
}

void AdminWindow::showStudentDetails(const Student& student) {
    const QString detail = QString::fromUtf8(
        u8"学号：%1\n姓名：%2\n专业：%3\n班级：%4\n入学年份：%5\n联系方式：%6")
            .arg(QString::fromStdString(student.getId()))
            .arg(QString::fromStdString(student.getName()))
            .arg(QString::fromStdString(student.getMajor()))
            .arg(QString::fromStdString(student.getClassName()))
            .arg(student.getEnrollYear())
            .arg(QString::fromStdString(student.getContact()));
    QMessageBox::information(this, QString::fromUtf8(u8"学生详情"), detail);
}

QString AdminWindow::selectedStudentId() const {
    const int row = studentTable_ ? studentTable_->currentRow() : -1;
    if (row < 0) {
        return {};
    }
    const QTableWidgetItem* item = studentTable_->item(row, 0);
    return item ? item->text() : QString{};
}

QWidget* AdminWindow::createCoursePage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>课程管理</b>"), page);
    auto* introLabel = new QLabel(
        QString::fromUtf8(u8"可查看课程列表、按课程号查询、新增、编辑和删除课程。"),
        page);
    introLabel->setWordWrap(true);

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(introLabel);

    auto* lookupLayout = new QHBoxLayout();
    lookupLayout->addWidget(new QLabel(QString::fromUtf8(u8"按课程号查看"), page));
    courseLookupEdit_ = new QLineEdit(page);
    courseLookupEdit_->setPlaceholderText(QString::fromUtf8(u8"例如：C001"));
    lookupLayout->addWidget(courseLookupEdit_, 1);

    auto* viewButton = new QPushButton(QString::fromUtf8(u8"查看"), page);
    auto* refreshButton = new QPushButton(QString::fromUtf8(u8"刷新列表"), page);
    lookupLayout->addWidget(viewButton);
    lookupLayout->addWidget(refreshButton);
    rootLayout->addLayout(lookupLayout);

    auto* actionLayout = new QHBoxLayout();
    auto* createButton = new QPushButton(QString::fromUtf8(u8"新增课程"), page);
    auto* editButton = new QPushButton(QString::fromUtf8(u8"编辑选中"), page);
    auto* removeButton = new QPushButton(QString::fromUtf8(u8"删除选中"), page);
    actionLayout->addWidget(createButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(removeButton);
    actionLayout->addStretch();
    rootLayout->addLayout(actionLayout);

    courseTable_ = new QTableWidget(page);
    courseTable_->setColumnCount(5);
    courseTable_->setHorizontalHeaderLabels({
        QString::fromUtf8(u8"课程号"),
        QString::fromUtf8(u8"课程名"),
        QString::fromUtf8(u8"学分"),
        QString::fromUtf8(u8"授课教师编号"),
        QString::fromUtf8(u8"学期")
    });
    configureTable(courseTable_);
    rootLayout->addWidget(courseTable_, 1);

    connect(refreshButton, &QPushButton::clicked, this, [this] { refreshCourseTable(); });
    connect(viewButton, &QPushButton::clicked, this, [this] { showCourseById(); });
    connect(createButton, &QPushButton::clicked, this, [this] { createCourse(); });
    connect(editButton, &QPushButton::clicked, this, [this] { editSelectedCourse(); });
    connect(removeButton, &QPushButton::clicked, this, [this] { removeSelectedCourse(); });
    connect(courseLookupEdit_, &QLineEdit::returnPressed, this, [this] { showCourseById(); });
    connect(courseTable_, &QTableWidget::cellDoubleClicked, this, [this](int, int) {
        editSelectedCourse();
    });

    refreshCourseTable();
    return page;
}

void AdminWindow::refreshCourseTable() {
    try {
        const auto courses = appContext_.courseService.listAll(session_);
        populateCourseTable(courseTable_, courses);
        statusBar()->showMessage(
            QString::fromUtf8(u8"课程列表已刷新，共 %1 条。").arg(courses.size()),
            4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"刷新课程列表失败"), e);
    }
}

void AdminWindow::showCourseById() {
    const QString id = courseLookupEdit_->text().trimmed();
    if (id.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"缺少课程号"), QString::fromUtf8(u8"请先输入要查看的课程号。"));
        courseLookupEdit_->setFocus();
        return;
    }

    try {
        const Course course = appContext_.courseService.findById(session_, id.toStdString());
        showCourseDetails(course);
        statusBar()->showMessage(QString::fromUtf8(u8"已查看课程：%1").arg(id), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"查询课程失败"), e);
    }
}

void AdminWindow::createCourse() {
    CourseEditDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    try {
        const Course course = dialog.course();
        appContext_.courseService.create(session_, course);
        refreshCourseTable();
        courseLookupEdit_->setText(QString::fromStdString(course.getCourseId()));
        statusBar()->showMessage(
            QString::fromUtf8(u8"课程已新增：%1").arg(QString::fromStdString(course.getCourseId())),
            4000);
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"新增成功"),
            QString::fromUtf8(u8"课程 %1 已创建。").arg(QString::fromStdString(course.getCourseId())));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"新增课程失败"), e);
    }
}

void AdminWindow::editSelectedCourse() {
    const QString courseId = selectedCourseId();
    if (courseId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"未选择课程"), QString::fromUtf8(u8"请先在表格中选择一门课程。"));
        return;
    }

    try {
        const Course current = appContext_.courseService.findById(session_, courseId.toStdString());
        CourseEditDialog dialog(current, this);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        const Course updated = dialog.course();
        appContext_.courseService.update(session_, updated);
        refreshCourseTable();
        courseLookupEdit_->setText(courseId);
        statusBar()->showMessage(QString::fromUtf8(u8"课程已更新：%1").arg(courseId), 4000);
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"更新成功"),
            QString::fromUtf8(u8"课程 %1 已更新。").arg(courseId));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"编辑课程失败"), e);
    }
}

void AdminWindow::removeSelectedCourse() {
    const QString courseId = selectedCourseId();
    if (courseId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"未选择课程"), QString::fromUtf8(u8"请先在表格中选择要删除的课程。"));
        return;
    }

    const auto confirm = QMessageBox::warning(
        this,
        QString::fromUtf8(u8"确认级联删除"),
        QString::fromUtf8(u8"删除课程 %1 后，将同时删除该课程关联的所有成绩记录。\n此操作不可撤销，是否继续？")
            .arg(courseId),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (confirm != QMessageBox::Yes) {
        return;
    }

    try {
        appContext_.courseService.remove(session_, courseId.toStdString());
        refreshCourseTable();
        courseLookupEdit_->clear();
        statusBar()->showMessage(QString::fromUtf8(u8"课程已删除：%1").arg(courseId), 4000);
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"删除成功"),
            QString::fromUtf8(u8"课程 %1 已级联删除。").arg(courseId));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"删除课程失败"), e);
    }
}

void AdminWindow::showCourseDetails(const Course& course) {
    const QString detail = QString::fromUtf8(
        u8"课程号：%1\n课程名：%2\n学分：%3\n授课教师编号：%4\n学期：%5")
            .arg(QString::fromStdString(course.getCourseId()))
            .arg(QString::fromStdString(course.getCourseName()))
            .arg(course.getCredit(), 0, 'f', 1)
            .arg(QString::fromStdString(course.getTeacherId()))
            .arg(QString::fromStdString(course.getSemester()));
    QMessageBox::information(this, QString::fromUtf8(u8"课程详情"), detail);
}

QString AdminWindow::selectedCourseId() const {
    const int row = courseTable_ ? courseTable_->currentRow() : -1;
    if (row < 0) {
        return {};
    }
    const QTableWidgetItem* item = courseTable_->item(row, 0);
    return item ? item->text() : QString{};
}

QWidget* AdminWindow::createScorePage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>成绩管理</b>"), page);
    auto* introLabel = new QLabel(
        QString::fromUtf8(u8"可查看成绩总览、按学生查询、按课程查询、录入更新和删除单条成绩。"),
        page);
    introLabel->setWordWrap(true);

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(introLabel);

    auto* filterLayout = new QHBoxLayout();
    auto* refreshButton = new QPushButton(QString::fromUtf8(u8"查看全部"), page);
    filterLayout->addWidget(refreshButton);

    filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"学生学号"), page));
    scoreStudentLookupEdit_ = new QLineEdit(page);
    scoreStudentLookupEdit_->setPlaceholderText(QString::fromUtf8(u8"例如：S001"));
    filterLayout->addWidget(scoreStudentLookupEdit_, 1);
    auto* byStudentButton = new QPushButton(QString::fromUtf8(u8"按学生查"), page);
    filterLayout->addWidget(byStudentButton);

    filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"课程号"), page));
    scoreCourseLookupEdit_ = new QLineEdit(page);
    scoreCourseLookupEdit_->setPlaceholderText(QString::fromUtf8(u8"例如：C001"));
    filterLayout->addWidget(scoreCourseLookupEdit_, 1);
    auto* byCourseButton = new QPushButton(QString::fromUtf8(u8"按课程查"), page);
    filterLayout->addWidget(byCourseButton);
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

    connect(refreshButton, &QPushButton::clicked, this, [this] { refreshScoreTable(); });
    connect(byStudentButton, &QPushButton::clicked, this, [this] { showScoresByStudent(); });
    connect(byCourseButton, &QPushButton::clicked, this, [this] { showScoresByCourse(); });
    connect(scoreStudentLookupEdit_, &QLineEdit::returnPressed, this, [this] { showScoresByStudent(); });
    connect(scoreCourseLookupEdit_, &QLineEdit::returnPressed, this, [this] { showScoresByCourse(); });
    connect(createButton, &QPushButton::clicked, this, [this] { createScore(); });
    connect(editButton, &QPushButton::clicked, this, [this] { editSelectedScore(); });
    connect(removeButton, &QPushButton::clicked, this, [this] { removeSelectedScore(); });
    connect(scoreTable_, &QTableWidget::cellDoubleClicked, this, [this](int, int) {
        editSelectedScore();
    });

    refreshScoreTable();
    return page;
}

void AdminWindow::refreshScoreTable() {
    try {
        const auto scores = appContext_.scoreService.listAll(session_);
        populateScoreTable(scoreTable_, scores);
        statusBar()->showMessage(
            QString::fromUtf8(u8"成绩列表已刷新，共 %1 条。").arg(scores.size()),
            4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"刷新成绩列表失败"), e);
    }
}

void AdminWindow::showScoresByStudent() {
    const QString studentId = scoreStudentLookupEdit_->text().trimmed();
    if (studentId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"缺少学号"), QString::fromUtf8(u8"请先输入要查询的学生学号。"));
        scoreStudentLookupEdit_->setFocus();
        return;
    }

    try {
        const auto scores = appContext_.scoreService.findByStudent(session_, studentId.toStdString());
        populateScoreTable(scoreTable_, scores);
        statusBar()->showMessage(
            QString::fromUtf8(u8"已按学生 %1 过滤成绩，共 %2 条。").arg(studentId).arg(scores.size()),
            4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"按学生查询成绩失败"), e);
    }
}

void AdminWindow::showScoresByCourse() {
    const QString courseId = scoreCourseLookupEdit_->text().trimmed();
    if (courseId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"缺少课程号"), QString::fromUtf8(u8"请先输入要查询的课程号。"));
        scoreCourseLookupEdit_->setFocus();
        return;
    }

    try {
        const auto scores = appContext_.scoreService.findByCourse(session_, courseId.toStdString());
        populateScoreTable(scoreTable_, scores);
        statusBar()->showMessage(
            QString::fromUtf8(u8"已按课程 %1 过滤成绩，共 %2 条。").arg(courseId).arg(scores.size()),
            4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"按课程查询成绩失败"), e);
    }
}

void AdminWindow::createScore() {
    ScoreEditDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    try {
        const Score score = dialog.score();
        appContext_.scoreService.upsert(session_, score);
        refreshScoreTable();
        scoreStudentLookupEdit_->setText(QString::fromStdString(score.getStudentId()));
        scoreCourseLookupEdit_->setText(QString::fromStdString(score.getCourseId()));
        statusBar()->showMessage(
            QString::fromUtf8(u8"成绩已写入：%1 / %2 / %3")
                .arg(QString::fromStdString(score.getStudentId()))
                .arg(QString::fromStdString(score.getCourseId()))
                .arg(QString::fromStdString(score.getSemester())),
            4000);
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"保存成功"),
            QString::fromUtf8(u8"成绩记录已写入或更新。"));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"保存成绩失败"), e);
    }
}

void AdminWindow::editSelectedScore() {
    QString studentId;
    QString courseId;
    QString semester;
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
        refreshScoreTable();
        statusBar()->showMessage(
            QString::fromUtf8(u8"成绩已更新：%1 / %2 / %3").arg(studentId).arg(courseId).arg(semester),
            4000);
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"更新成功"),
            QString::fromUtf8(u8"成绩记录已更新。"));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"更新成绩失败"), e);
    }
}

void AdminWindow::removeSelectedScore() {
    QString studentId;
    QString courseId;
    QString semester;
    if (!selectedScoreKey(studentId, courseId, semester)) {
        QMessageBox::information(this, QString::fromUtf8(u8"未选择成绩"), QString::fromUtf8(u8"请先在表格中选择要删除的成绩记录。"));
        return;
    }

    const auto confirm = QMessageBox::warning(
        this,
        QString::fromUtf8(u8"确认删除"),
        QString::fromUtf8(u8"将删除成绩 %1 / %2 / %3，是否继续？")
            .arg(studentId)
            .arg(courseId)
            .arg(semester),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (confirm != QMessageBox::Yes) {
        return;
    }

    try {
        appContext_.scoreService.remove(
            session_,
            studentId.toStdString(),
            courseId.toStdString(),
            semester.toStdString());
        refreshScoreTable();
        statusBar()->showMessage(
            QString::fromUtf8(u8"成绩已删除：%1 / %2 / %3").arg(studentId).arg(courseId).arg(semester),
            4000);
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"删除成功"),
            QString::fromUtf8(u8"成绩记录已删除。"));
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"删除成绩失败"), e);
    }
}

bool AdminWindow::selectedScoreKey(QString& studentId, QString& courseId, QString& semester) const {
    const int row = scoreTable_ ? scoreTable_->currentRow() : -1;
    if (row < 0) {
        return false;
    }

    const QTableWidgetItem* studentItem = scoreTable_->item(row, 0);
    const QTableWidgetItem* courseItem = scoreTable_->item(row, 1);
    const QTableWidgetItem* semesterItem = scoreTable_->item(row, 2);
    if (!studentItem || !courseItem || !semesterItem) {
        return false;
    }

    studentId = studentItem->text();
    courseId = courseItem->text();
    semester = semesterItem->text();
    return true;
}

void AdminWindow::showServiceError(const QString& title, const std::exception& e) {
    QMessageBox::critical(this, title, QString::fromLocal8Bit(e.what()));
}

// ---------------------------------------------------------------------------
// Account page
// ---------------------------------------------------------------------------

QWidget* AdminWindow::createAccountPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>账户</b>"), page);
    auto* infoLabel = new QLabel(
        QString::fromUtf8(u8"当前用户：%1（角色：管理员）")
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

// ---------------------------------------------------------------------------
// Stats page
// ---------------------------------------------------------------------------

QWidget* AdminWindow::createStatsPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>统计分析</b>"), page);
    rootLayout->addWidget(titleLabel);

    // --- Course stats section ---
    auto* statsGroup = new QHBoxLayout();
    statsGroup->addWidget(new QLabel(QString::fromUtf8(u8"课程号"), page));
    statsCourseEdit_ = new QLineEdit(page);
    statsCourseEdit_->setPlaceholderText(QString::fromUtf8(u8"例如：C001"));
    statsGroup->addWidget(statsCourseEdit_, 1);
    auto* courseStatsButton = new QPushButton(QString::fromUtf8(u8"课程统计"), page);
    auto* courseRankButton = new QPushButton(QString::fromUtf8(u8"课程排名"), page);
    statsGroup->addWidget(courseStatsButton);
    statsGroup->addWidget(courseRankButton);
    rootLayout->addLayout(statsGroup);

    // --- Student GPA section ---
    auto* gpaGroup = new QHBoxLayout();
    gpaGroup->addWidget(new QLabel(QString::fromUtf8(u8"学号"), page));
    statsStudentEdit_ = new QLineEdit(page);
    statsStudentEdit_->setPlaceholderText(QString::fromUtf8(u8"例如：S001"));
    gpaGroup->addWidget(statsStudentEdit_, 1);
    auto* gpaButton = new QPushButton(QString::fromUtf8(u8"查询 GPA"), page);
    gpaGroup->addWidget(gpaButton);
    rootLayout->addLayout(gpaGroup);

    // --- Ranking table ---
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
    connect(gpaButton, &QPushButton::clicked, this, [this] { queryStudentGpa(); });
    connect(statsCourseEdit_, &QLineEdit::returnPressed, this, [this] { queryCourseStats(); });
    connect(statsStudentEdit_, &QLineEdit::returnPressed, this, [this] { queryStudentGpa(); });

    return page;
}

void AdminWindow::queryCourseStats() {
    const QString courseId = statsCourseEdit_->text().trimmed();
    if (courseId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"缺少课程号"), QString::fromUtf8(u8"请先输入课程号。"));
        statsCourseEdit_->setFocus();
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
        statusBar()->showMessage(
            QString::fromUtf8(u8"已查询课程 %1 统计。").arg(courseId), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"查询课程统计失败"), e);
    }
}

void AdminWindow::queryCourseRanking() {
    const QString courseId = statsCourseEdit_->text().trimmed();
    if (courseId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"缺少课程号"), QString::fromUtf8(u8"请先输入课程号。"));
        statsCourseEdit_->setFocus();
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
            QString::fromUtf8(u8"课程 %1 排名已加载，共 %2 人。").arg(courseId).arg(ranking.size()),
            4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"查询课程排名失败"), e);
    }
}

void AdminWindow::queryStudentGpa() {
    const QString studentId = statsStudentEdit_->text().trimmed();
    if (studentId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"缺少学号"), QString::fromUtf8(u8"请先输入学号。"));
        statsStudentEdit_->setFocus();
        return;
    }

    try {
        const GpaResult gpa = appContext_.statsService.computeGpaFor(session_, studentId.toStdString());
        const QString detail = QString::fromUtf8(
            u8"学号：%1\n"
            u8"姓名：%2\n"
            u8"已修课程数：%3\n"
            u8"总学分：%4\n"
            u8"GPA：%5")
                .arg(QString::fromStdString(gpa.studentId))
                .arg(QString::fromStdString(gpa.studentName))
                .arg(gpa.courseCount)
                .arg(gpa.totalCredit, 0, 'f', 1)
                .arg(gpa.gpa, 0, 'f', 3);
        QMessageBox::information(this, QString::fromUtf8(u8"学生 GPA"), detail);
        statusBar()->showMessage(
            QString::fromUtf8(u8"已查询学生 %1 GPA。").arg(studentId), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"查询学生 GPA 失败"), e);
    }
}

// ---------------------------------------------------------------------------
// Report export page
// ---------------------------------------------------------------------------

QWidget* AdminWindow::createReportPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>报告导出</b>"), page);
    auto* introLabel = new QLabel(
        QString::fromUtf8(u8"导出学业预警报告（.txt）和课程统计/排名（.csv）到 data/ 目录。"),
        page);
    introLabel->setWordWrap(true);
    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(introLabel);

    auto* warningButton = new QPushButton(QString::fromUtf8(u8"生成学业预警报告 (warning_report.txt)"), page);
    rootLayout->addWidget(warningButton);

    rootLayout->addSpacing(16);

    auto* csvGroup = new QHBoxLayout();
    csvGroup->addWidget(new QLabel(QString::fromUtf8(u8"课程号"), page));
    reportCourseEdit_ = new QLineEdit(page);
    reportCourseEdit_->setPlaceholderText(QString::fromUtf8(u8"例如：C001"));
    csvGroup->addWidget(reportCourseEdit_, 1);
    auto* statsCsvButton = new QPushButton(QString::fromUtf8(u8"导出课程统计 CSV"), page);
    auto* rankCsvButton = new QPushButton(QString::fromUtf8(u8"导出课程排名 CSV"), page);
    csvGroup->addWidget(statsCsvButton);
    csvGroup->addWidget(rankCsvButton);
    rootLayout->addLayout(csvGroup);

    rootLayout->addStretch();

    connect(warningButton, &QPushButton::clicked, this, [this] { exportWarningReport(); });
    connect(statsCsvButton, &QPushButton::clicked, this, [this] { exportCourseStatsCsv(); });
    connect(rankCsvButton, &QPushButton::clicked, this, [this] { exportRankingCsv(); });

    return page;
}

void AdminWindow::exportWarningReport() {
    try {
        const std::string path = appContext_.reportExporter.exportWarningReport(session_);
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"导出成功"),
            QString::fromUtf8(u8"学业预警报告已写入：\n%1").arg(QString::fromStdString(path)));
        statusBar()->showMessage(QString::fromUtf8(u8"预警报告已导出。"), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"导出预警报告失败"), e);
    }
}

void AdminWindow::exportCourseStatsCsv() {
    const QString courseId = reportCourseEdit_->text().trimmed();
    if (courseId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"缺少课程号"), QString::fromUtf8(u8"请先输入课程号。"));
        reportCourseEdit_->setFocus();
        return;
    }

    try {
        const std::string path = appContext_.reportExporter.exportCourseStatsCsv(session_, courseId.toStdString());
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"导出成功"),
            QString::fromUtf8(u8"课程统计 CSV 已写入：\n%1").arg(QString::fromStdString(path)));
        statusBar()->showMessage(
            QString::fromUtf8(u8"课程 %1 统计 CSV 已导出。").arg(courseId), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"导出课程统计 CSV 失败"), e);
    }
}

void AdminWindow::exportRankingCsv() {
    const QString courseId = reportCourseEdit_->text().trimmed();
    if (courseId.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8(u8"缺少课程号"), QString::fromUtf8(u8"请先输入课程号。"));
        reportCourseEdit_->setFocus();
        return;
    }

    try {
        const std::string path = appContext_.reportExporter.exportRankingCsv(session_, courseId.toStdString());
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"导出成功"),
            QString::fromUtf8(u8"课程排名 CSV 已写入：\n%1").arg(QString::fromStdString(path)));
        statusBar()->showMessage(
            QString::fromUtf8(u8"课程 %1 排名 CSV 已导出。").arg(courseId), 4000);
    } catch (const std::exception& e) {
        showServiceError(QString::fromUtf8(u8"导出课程排名 CSV 失败"), e);
    }
}

} // namespace EduSys
