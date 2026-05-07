#include "EduSys/gui/LoginDialog.hpp"

#include <exception>
#include <string>

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "EduSys/app/AppContext.hpp"
#include "EduSys/common/Exception.hpp"
#include "EduSys/model/UserAccount.hpp"

namespace {

QString errorText(const std::exception& e) {
    return QString::fromLocal8Bit(e.what());
}

} // namespace

namespace EduSys {

LoginDialog::LoginDialog(AppContext& appContext, QWidget* parent)
    : QDialog(parent)
    , appContext_(appContext) {
    setWindowTitle(QString::fromUtf8(u8"EduSys 登录"));
    setModal(true);
    setMinimumWidth(420);

    auto* rootLayout = new QVBoxLayout(this);

    auto* introLabel = new QLabel(
        QString::fromUtf8(u8"请输入用户名和密码。连续 3 次认证失败后，程序会按控制台版规则退出。"),
        this);
    introLabel->setWordWrap(true);
    rootLayout->addWidget(introLabel);

    auto* formLayout = new QFormLayout();
    usernameEdit_ = new QLineEdit(this);
    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setEchoMode(QLineEdit::Password);

    formLayout->addRow(QString::fromUtf8(u8"用户名"), usernameEdit_);
    formLayout->addRow(QString::fromUtf8(u8"密码"), passwordEdit_);
    rootLayout->addLayout(formLayout);

    auto* buttons = new QDialogButtonBox(this);
    QPushButton* loginButton = buttons->addButton(QString::fromUtf8(u8"登录"), QDialogButtonBox::AcceptRole);
    buttons->addButton(QString::fromUtf8(u8"退出"), QDialogButtonBox::RejectRole);
    rootLayout->addWidget(buttons);

    connect(loginButton, &QPushButton::clicked, this, [this] { tryLogin(); });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(usernameEdit_, &QLineEdit::returnPressed, this, [this] {
        passwordEdit_->setFocus();
    });
    connect(passwordEdit_, &QLineEdit::returnPressed, this, [this] { tryLogin(); });

    usernameEdit_->setFocus();
}

void LoginDialog::tryLogin() {
    const QString username = usernameEdit_->text().trimmed();
    const QString password = passwordEdit_->text();

    if (username.isEmpty()) {
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"结束登录"),
            QString::fromUtf8(u8"用户名为空，本次启动将直接退出。"));
        reject();
        return;
    }

    try {
        const UserAccount account =
            appContext_.authService.authenticate(username.toStdString(), password.toStdString());
        session_.login(account.getUsername(), account.getRole(), account.getOwnerId());
        accept();
    } catch (const AuthException& e) {
        ++failedAttempts_;
        QMessageBox::warning(
            this,
            QString::fromUtf8(u8"登录失败"),
            QString::fromUtf8(u8"%1\n\n当前失败次数：%2 / 3")
                .arg(errorText(e))
                .arg(failedAttempts_));

        passwordEdit_->clear();
        passwordEdit_->setFocus();

        if (failedAttempts_ >= 3) {
            QMessageBox::critical(
                this,
                QString::fromUtf8(u8"登录终止"),
                QString::fromUtf8(u8"连续 3 次认证失败，程序将退出。"));
            reject();
        }
    } catch (const EduException& e) {
        QMessageBox::critical(
            this,
            QString::fromUtf8(u8"业务错误"),
            errorText(e));
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this,
            QString::fromUtf8(u8"未知错误"),
            errorText(e));
    }
}

} // namespace EduSys
