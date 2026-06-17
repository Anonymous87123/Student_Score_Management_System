#pragma once

#include <QDialog>

#include "EduSys/service/Session.hpp"

class QLineEdit;

namespace EduSys {

class AppContext;

class LoginDialog : public QDialog {
public:
    explicit LoginDialog(AppContext& appContext, QWidget* parent = nullptr);

    const Session& session() const noexcept { return session_; }

private:
    void tryLogin();
    void changePasswordBeforeLogin();

    AppContext& appContext_;
    Session     session_;
    QLineEdit*  usernameEdit_ = nullptr;
    QLineEdit*  passwordEdit_ = nullptr;
    int         failedAttempts_ = 0;
};

} // namespace EduSys
