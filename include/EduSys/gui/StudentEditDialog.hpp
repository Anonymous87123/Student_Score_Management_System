#pragma once

#include <QDialog>

#include "EduSys/model/Student.hpp"

class QLineEdit;
class QSpinBox;

namespace EduSys {

class StudentEditDialog : public QDialog {
public:
    explicit StudentEditDialog(QWidget* parent = nullptr);
    StudentEditDialog(const Student& student, QWidget* parent = nullptr);

    Student student() const;

private:
    void setupUi();
    void loadStudent(const Student& student);
    void validateAndAccept();

    bool       editing_ = false;
    QLineEdit* idEdit_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QLineEdit* contactEdit_ = nullptr;
    QLineEdit* majorEdit_ = nullptr;
    QLineEdit* classEdit_ = nullptr;
    QSpinBox*  yearSpin_ = nullptr;
};

} // namespace EduSys
