#include "EduSys/gui/ChangePasswordDialog.hpp"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

#include "EduSys/app/AppContext.hpp"
#include "EduSys/service/Session.hpp"

namespace EduSys {

ChangePasswordDialog::ChangePasswordDialog(AppContext& appContext, const Session& session, QWidget* parent)
    : QDialog(parent)
    , appContext_(appContext)
    , session_(session) {
    setWindowTitle(QString::fromUtf8(u8"修改密码"));
    setModal(true);
    setMinimumWidth(380);

    auto* rootLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    oldPwEdit_ = new QLineEdit(this);
    oldPwEdit_->setEchoMode(QLineEdit::Password);
    newPwEdit_ = new QLineEdit(this);
    newPwEdit_->setEchoMode(QLineEdit::Password);
    confirmEdit_ = new QLineEdit(this);
    confirmEdit_->setEchoMode(QLineEdit::Password);

    formLayout->addRow(QString::fromUtf8(u8"当前密码"), oldPwEdit_);
    formLayout->addRow(QString::fromUtf8(u8"新密码"), newPwEdit_);
    formLayout->addRow(QString::fromUtf8(u8"确认新密码"), confirmEdit_);
    rootLayout->addLayout(formLayout);

    auto* buttons = new QDialogButtonBox(this);
    buttons->addButton(QString::fromUtf8(u8"确认修改"), QDialogButtonBox::AcceptRole);
    buttons->addButton(QString::fromUtf8(u8"取消"), QDialogButtonBox::RejectRole);
    rootLayout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this] { validateAndAccept(); });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void ChangePasswordDialog::validateAndAccept() {
    const QString oldPw = oldPwEdit_->text();
    const QString newPw = newPwEdit_->text();
    const QString confirm = confirmEdit_->text();

    if (oldPw.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"请输入当前密码。"));
        oldPwEdit_->setFocus();
        return;
    }
    if (newPw.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"请输入新密码。"));
        newPwEdit_->setFocus();
        return;
    }
    if (newPw != confirm) {
        QMessageBox::warning(this, QString::fromUtf8(u8"密码不匹配"), QString::fromUtf8(u8"两次输入的新密码不一致。"));
        confirmEdit_->setFocus();
        return;
    }

    try {
        appContext_.authService.changePassword(session_, oldPw.toStdString(), newPw.toStdString());
        QMessageBox::information(this, QString::fromUtf8(u8"修改成功"), QString::fromUtf8(u8"密码已更新。"));
        accept();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, QString::fromUtf8(u8"修改失败"), QString::fromLocal8Bit(e.what()));
    }
}

} // namespace EduSys
