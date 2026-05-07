#pragma once

#include <exception>

#include <QMainWindow>

#include "EduSys/service/Session.hpp"

class QLineEdit;
class QTableWidget;

namespace EduSys {

class AppContext;
class Student;

class AdminWindow : public QMainWindow {
public:
    AdminWindow(AppContext& appContext, Session session, QWidget* parent = nullptr);

private:
    QWidget* createStudentPage();
    QWidget* createPlaceholderPage(const QString& title, const QString& summary);
    void refreshStudentTable();
    void showStudentById();
    void createStudent();
    void editSelectedStudent();
    void removeSelectedStudent();
    void showStudentDetails(const Student& student);
    QString selectedStudentId() const;
    void showServiceError(const QString& title, const std::exception& e);

    AppContext& appContext_;
    Session     session_;
    QLineEdit*  studentLookupEdit_ = nullptr;
    QTableWidget* studentTable_ = nullptr;
};

} // namespace EduSys
