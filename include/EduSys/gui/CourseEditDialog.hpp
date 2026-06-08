#pragma once

#include <QDialog>

#include "EduSys/model/Course.hpp"

class QLineEdit;
class QDoubleSpinBox;

namespace EduSys {

class CourseEditDialog : public QDialog {
public:
    explicit CourseEditDialog(QWidget* parent = nullptr);
    CourseEditDialog(const Course& course, QWidget* parent = nullptr);

    Course course() const;

private:
    void setupUi();
    void loadCourse(const Course& course);
    void validateAndAccept();

    bool            editing_ = false;
    QLineEdit*      idEdit_ = nullptr;
    QLineEdit*      nameEdit_ = nullptr;
    QDoubleSpinBox* creditSpin_ = nullptr;
    QLineEdit*      teacherIdEdit_ = nullptr;
    QLineEdit*      semesterEdit_ = nullptr;
};

} // namespace EduSys
