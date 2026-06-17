#include "EduSys/gui/TeacherEditDialog.hpp"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

namespace EduSys {

TeacherEditDialog::TeacherEditDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    setWindowTitle(QString::fromUtf8(u8"新增教师"));
}

TeacherEditDialog::TeacherEditDialog(const Teacher& teacher, QWidget* parent)
    : QDialog(parent)
    , editing_(true) {
    setupUi();
    setWindowTitle(QString::fromUtf8(u8"编辑教师"));
    loadTeacher(teacher);
}

Teacher TeacherEditDialog::teacher() const {
    return Teacher(
        idEdit_->text().trimmed().toStdString(),
        nameEdit_->text().trimmed().toStdString(),
        contactEdit_->text().trimmed().toStdString(),
        departmentEdit_->text().trimmed().toStdString(),
        titleEdit_->text().trimmed().toStdString());
}

std::string TeacherEditDialog::loginUsername() const {
    return usernameEdit_ ? usernameEdit_->text().trimmed().toStdString() : std::string{};
}

std::string TeacherEditDialog::initialPassword() const {
    return passwordEdit_ ? passwordEdit_->text().toStdString() : std::string{};
}

void TeacherEditDialog::setupUi() {
    setModal(true);
    setMinimumWidth(440);

    auto* rootLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    idEdit_ = new QLineEdit(this);
    nameEdit_ = new QLineEdit(this);
    contactEdit_ = new QLineEdit(this);
    departmentEdit_ = new QLineEdit(this);
    titleEdit_ = new QLineEdit(this);
    usernameEdit_ = new QLineEdit(this);
    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setEchoMode(QLineEdit::Password);

    formLayout->addRow(QString::fromUtf8(u8"教师编号"), idEdit_);
    formLayout->addRow(QString::fromUtf8(u8"姓名"), nameEdit_);
    formLayout->addRow(QString::fromUtf8(u8"联系方式"), contactEdit_);
    formLayout->addRow(QString::fromUtf8(u8"院系"), departmentEdit_);
    formLayout->addRow(QString::fromUtf8(u8"职称"), titleEdit_);
    rootLayout->addLayout(formLayout);

    auto* accountHint = new QLabel(
        QString::fromUtf8(u8"登录账号仅在新增教师时创建；留空表示只新增教师资料。"),
        this);
    accountHint->setWordWrap(true);
    rootLayout->addWidget(accountHint);

    auto* accountLayout = new QFormLayout();
    accountLayout->addRow(QString::fromUtf8(u8"登录用户名"), usernameEdit_);
    accountLayout->addRow(QString::fromUtf8(u8"初始密码"), passwordEdit_);
    rootLayout->addLayout(accountLayout);

    auto* buttons = new QDialogButtonBox(this);
    buttons->addButton(QString::fromUtf8(u8"保存"), QDialogButtonBox::AcceptRole);
    buttons->addButton(QString::fromUtf8(u8"取消"), QDialogButtonBox::RejectRole);
    rootLayout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this] { validateAndAccept(); });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void TeacherEditDialog::loadTeacher(const Teacher& teacher) {
    idEdit_->setText(QString::fromStdString(teacher.getId()));
    idEdit_->setEnabled(false);
    nameEdit_->setText(QString::fromStdString(teacher.getName()));
    contactEdit_->setText(QString::fromStdString(teacher.getContact()));
    departmentEdit_->setText(QString::fromStdString(teacher.getDepartment()));
    titleEdit_->setText(QString::fromStdString(teacher.getTitle()));
    usernameEdit_->setEnabled(false);
    passwordEdit_->setEnabled(false);
    usernameEdit_->setPlaceholderText(QString::fromUtf8(u8"编辑教师资料时不修改账号"));
}

void TeacherEditDialog::validateAndAccept() {
    if (idEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"教师编号不能为空。"));
        idEdit_->setFocus();
        return;
    }
    if (nameEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"姓名不能为空。"));
        nameEdit_->setFocus();
        return;
    }
    if (!editing_ &&
        !usernameEdit_->text().trimmed().isEmpty() &&
        passwordEdit_->text().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"填写登录用户名时，初始密码不能为空。"));
        passwordEdit_->setFocus();
        return;
    }
    accept();
}

} // namespace EduSys
