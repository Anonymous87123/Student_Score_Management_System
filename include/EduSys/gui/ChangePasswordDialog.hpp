#pragma once

#include <QDialog>

class QLineEdit;

namespace EduSys {

class AppContext;
class Session;

class ChangePasswordDialog : public QDialog {
public:
    ChangePasswordDialog(AppContext& appContext, const Session& session, QWidget* parent = nullptr);

private:
    void validateAndAccept();

    AppContext&    appContext_;
    const Session& session_;
    QLineEdit*    oldPwEdit_ = nullptr;
    QLineEdit*    newPwEdit_ = nullptr;
    QLineEdit*    confirmEdit_ = nullptr;
};

} // namespace EduSys
