#include "EduSys/gui/CourseEditDialog.hpp"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

namespace EduSys {

CourseEditDialog::CourseEditDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    setWindowTitle(QString::fromUtf8(u8"新增课程"));
}

CourseEditDialog::CourseEditDialog(const Course& course, QWidget* parent)
    : QDialog(parent)
    , editing_(true) {
    setupUi();
    setWindowTitle(QString::fromUtf8(u8"编辑课程"));
    loadCourse(course);
}

Course CourseEditDialog::course() const {
    return Course(
        idEdit_->text().trimmed().toStdString(),
        nameEdit_->text().trimmed().toStdString(),
        creditSpin_->value(),
        teacherIdEdit_->text().trimmed().toStdString(),
        semesterEdit_->text().trimmed().toStdString());
}

void CourseEditDialog::setupUi() {
    setModal(true);
    setMinimumWidth(420);

    auto* rootLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    idEdit_ = new QLineEdit(this);
    nameEdit_ = new QLineEdit(this);
    creditSpin_ = new QDoubleSpinBox(this);
    creditSpin_->setRange(0.5, 20.0);
    creditSpin_->setSingleStep(0.5);
    creditSpin_->setDecimals(1);
    creditSpin_->setValue(2.0);
    teacherIdEdit_ = new QLineEdit(this);
    teacherIdEdit_->setPlaceholderText(QString::fromUtf8(u8"例如：T001 或 T001,T002"));
    semesterEdit_ = new QLineEdit(this);
    semesterEdit_->setPlaceholderText(QString::fromUtf8(u8"例如：2025-1"));

    formLayout->addRow(QString::fromUtf8(u8"课程号"), idEdit_);
    formLayout->addRow(QString::fromUtf8(u8"课程名"), nameEdit_);
    formLayout->addRow(QString::fromUtf8(u8"学分"), creditSpin_);
    formLayout->addRow(QString::fromUtf8(u8"授课教师编号"), teacherIdEdit_);
    formLayout->addRow(QString::fromUtf8(u8"学期"), semesterEdit_);
    rootLayout->addLayout(formLayout);

    auto* buttons = new QDialogButtonBox(this);
    buttons->addButton(QString::fromUtf8(u8"保存"), QDialogButtonBox::AcceptRole);
    buttons->addButton(QString::fromUtf8(u8"取消"), QDialogButtonBox::RejectRole);
    rootLayout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this] { validateAndAccept(); });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void CourseEditDialog::loadCourse(const Course& course) {
    idEdit_->setText(QString::fromStdString(course.getCourseId()));
    idEdit_->setEnabled(false);
    nameEdit_->setText(QString::fromStdString(course.getCourseName()));
    creditSpin_->setValue(course.getCredit());
    teacherIdEdit_->setText(QString::fromStdString(course.getTeacherId()));
    semesterEdit_->setText(QString::fromStdString(course.getSemester()));
}

void CourseEditDialog::validateAndAccept() {
    if (idEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"课程号不能为空。"));
        idEdit_->setFocus();
        return;
    }
    if (nameEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"课程名不能为空。"));
        nameEdit_->setFocus();
        return;
    }
    if (teacherIdEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"授课教师编号不能为空；多个教师可用英文逗号分隔。"));
        teacherIdEdit_->setFocus();
        return;
    }
    accept();
}

} // namespace EduSys
