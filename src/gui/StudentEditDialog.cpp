#include "EduSys/gui/StudentEditDialog.hpp"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

namespace EduSys {

StudentEditDialog::StudentEditDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    setWindowTitle(QString::fromUtf8(u8"新增学生"));
}

StudentEditDialog::StudentEditDialog(const Student& student, QWidget* parent)
    : QDialog(parent)
    , editing_(true) {
    setupUi();
    setWindowTitle(QString::fromUtf8(u8"编辑学生"));
    loadStudent(student);
}

Student StudentEditDialog::student() const {
    return Student(
        idEdit_->text().trimmed().toStdString(),
        nameEdit_->text().trimmed().toStdString(),
        contactEdit_->text().trimmed().toStdString(),
        majorEdit_->text().trimmed().toStdString(),
        classEdit_->text().trimmed().toStdString(),
        yearSpin_->value());
}

void StudentEditDialog::setupUi() {
    setModal(true);
    setMinimumWidth(420);

    auto* rootLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    idEdit_ = new QLineEdit(this);
    nameEdit_ = new QLineEdit(this);
    contactEdit_ = new QLineEdit(this);
    majorEdit_ = new QLineEdit(this);
    classEdit_ = new QLineEdit(this);
    yearSpin_ = new QSpinBox(this);
    yearSpin_->setRange(1900, 2100);
    yearSpin_->setValue(2025);

    formLayout->addRow(QString::fromUtf8(u8"学号"), idEdit_);
    formLayout->addRow(QString::fromUtf8(u8"姓名"), nameEdit_);
    formLayout->addRow(QString::fromUtf8(u8"联系方式"), contactEdit_);
    formLayout->addRow(QString::fromUtf8(u8"专业"), majorEdit_);
    formLayout->addRow(QString::fromUtf8(u8"班级"), classEdit_);
    formLayout->addRow(QString::fromUtf8(u8"入学年份"), yearSpin_);
    rootLayout->addLayout(formLayout);

    auto* buttons = new QDialogButtonBox(this);
    buttons->addButton(QString::fromUtf8(u8"保存"), QDialogButtonBox::AcceptRole);
    buttons->addButton(QString::fromUtf8(u8"取消"), QDialogButtonBox::RejectRole);
    rootLayout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this] { validateAndAccept(); });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void StudentEditDialog::loadStudent(const Student& student) {
    idEdit_->setText(QString::fromStdString(student.getId()));
    idEdit_->setEnabled(false);
    nameEdit_->setText(QString::fromStdString(student.getName()));
    contactEdit_->setText(QString::fromStdString(student.getContact()));
    majorEdit_->setText(QString::fromStdString(student.getMajor()));
    classEdit_->setText(QString::fromStdString(student.getClassName()));
    yearSpin_->setValue(student.getEnrollYear());
}

void StudentEditDialog::validateAndAccept() {
    if (idEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"学号不能为空。"));
        idEdit_->setFocus();
        return;
    }
    if (nameEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"姓名不能为空。"));
        nameEdit_->setFocus();
        return;
    }
    accept();
}

} // namespace EduSys
