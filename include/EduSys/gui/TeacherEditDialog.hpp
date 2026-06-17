#pragma once

#include <string>

#include <QDialog>

#include "EduSys/model/Teacher.hpp"

class QLineEdit;

namespace EduSys {

class TeacherEditDialog : public QDialog {
public:
    explicit TeacherEditDialog(QWidget* parent = nullptr);
    TeacherEditDialog(const Teacher& teacher, QWidget* parent = nullptr);

    Teacher teacher() const;
    std::string loginUsername() const;
    std::string initialPassword() const;

private:
    void setupUi();
    void loadTeacher(const Teacher& teacher);
    void validateAndAccept();

    bool       editing_ = false;
    QLineEdit* idEdit_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QLineEdit* contactEdit_ = nullptr;
    QLineEdit* departmentEdit_ = nullptr;
    QLineEdit* titleEdit_ = nullptr;
    QLineEdit* usernameEdit_ = nullptr;
    QLineEdit* passwordEdit_ = nullptr;
};

} // namespace EduSys
