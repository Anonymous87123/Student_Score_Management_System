#include "EduSys/gui/AdminWindow.hpp"

#include <utility>
#include <vector>

#include <QAbstractItemView>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
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
#include "EduSys/common/Exception.hpp"
#include "EduSys/gui/StudentEditDialog.hpp"
#include "EduSys/model/Student.hpp"

namespace {

QWidget* createInfoPage(const QString& title, const QString& summary, QWidget* parent = nullptr) {
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

QTableWidgetItem* createReadOnlyItem(const QString& text) {
    auto* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
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
    tabs->addTab(
        createPlaceholderPage(
            QString::fromUtf8(u8"课程管理"),
            QString::fromUtf8(u8"下一阶段会接入课程列表、按课程号查看、新增、编辑和级联删除。")),
        QString::fromUtf8(u8"课程管理"));
    tabs->addTab(
        createPlaceholderPage(
            QString::fromUtf8(u8"成绩管理"),
            QString::fromUtf8(u8"下一阶段会接入全部成绩、按学生查、按课程查、录入更新与删除单条成绩。")),
        QString::fromUtf8(u8"成绩管理"));
    tabs->addTab(
        createPlaceholderPage(
            QString::fromUtf8(u8"统计分析"),
            QString::fromUtf8(u8"下一阶段会接入课程统计、课程排名和学生 GPA 的真实结果展示。")),
        QString::fromUtf8(u8"统计分析"));
    tabs->addTab(
        createPlaceholderPage(
            QString::fromUtf8(u8"报告导出"),
            QString::fromUtf8(u8"下一阶段会接入预警报告、课程统计 CSV 和课程排名 CSV 导出。")),
        QString::fromUtf8(u8"报告导出"));
    tabs->addTab(
        createPlaceholderPage(
            QString::fromUtf8(u8"账户"),
            QString::fromUtf8(u8"下一阶段会接入修改密码与退出登录。")),
        QString::fromUtf8(u8"账户"));

    setCentralWidget(tabs);
    statusBar()->showMessage(
        QString::fromUtf8(u8"当前用户：%1。学生管理页已接入真实 StudentService。")
            .arg(QString::fromStdString(session_.getUsername())));
}

QWidget* AdminWindow::createStudentPage() {
    auto* page = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(page);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"<b>学生管理</b>"), page);
    auto* introLabel = new QLabel(
        QString::fromUtf8(u8"本页已经接入真实业务层，可执行学生列表、按学号查看、新增、编辑和级联删除。"),
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
    studentTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    studentTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    studentTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    studentTable_->setAlternatingRowColors(true);
    studentTable_->horizontalHeader()->setStretchLastSection(true);
    studentTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    studentTable_->verticalHeader()->setVisible(false);
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

QWidget* AdminWindow::createPlaceholderPage(const QString& title, const QString& summary) {
    return createInfoPage(title, summary, this);
}

void AdminWindow::refreshStudentTable() {
    try {
        const std::vector<Student> students = appContext_.studentService.listAll(session_);
        studentTable_->setRowCount(static_cast<int>(students.size()));

        int row = 0;
        for (const auto& student : students) {
            studentTable_->setItem(row, 0, createReadOnlyItem(QString::fromStdString(student.getId())));
            studentTable_->setItem(row, 1, createReadOnlyItem(QString::fromStdString(student.getName())));
            studentTable_->setItem(row, 2, createReadOnlyItem(QString::fromStdString(student.getMajor())));
            studentTable_->setItem(row, 3, createReadOnlyItem(QString::fromStdString(student.getClassName())));
            studentTable_->setItem(row, 4, createReadOnlyItem(QString::number(student.getEnrollYear())));
            studentTable_->setItem(row, 5, createReadOnlyItem(QString::fromStdString(student.getContact())));
            ++row;
        }

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
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"缺少学号"),
            QString::fromUtf8(u8"请先输入要查看的学号。"));
        studentLookupEdit_->setFocus();
        return;
    }

    try {
        const Student student = appContext_.studentService.findById(session_, id.toStdString());
        showStudentDetails(student);
        statusBar()->showMessage(
            QString::fromUtf8(u8"已查看学生：%1").arg(id),
            4000);
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
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"未选择学生"),
            QString::fromUtf8(u8"请先在表格中选择一名学生。"));
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
        statusBar()->showMessage(
            QString::fromUtf8(u8"学生已更新：%1").arg(studentId),
            4000);
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

    const QMessageBox::StandardButton confirm = QMessageBox::warning(
        this,
        QString::fromUtf8(u8"确认级联删除"),
        QString::fromUtf8(
            u8"删除学生 %1 后，将同时删除该学生关联的成绩记录和学生账户。\n此操作不可撤销，是否继续？")
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
        statusBar()->showMessage(
            QString::fromUtf8(u8"学生已删除：%1").arg(studentId),
            4000);
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

void AdminWindow::showServiceError(const QString& title, const std::exception& e) {
    QMessageBox::critical(this, title, QString::fromLocal8Bit(e.what()));
}

} // namespace EduSys
