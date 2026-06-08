#pragma once

#include <exception>

#include <QMainWindow>
#include <QString>

#include "EduSys/service/Session.hpp"

class QLabel;
class QTableWidget;

namespace EduSys {

class AppContext;

class StudentWindow : public QMainWindow {
public:
    StudentWindow(AppContext& appContext, Session session, QWidget* parent = nullptr);

private:
    QWidget* createProfilePage();
    QWidget* createMyScoresPage();
    QWidget* createMyGpaPage();
    QWidget* createAccountPage();

    void refreshMyScores();
    void refreshMyGpa();

    void showServiceError(const QString& title, const std::exception& e);

    AppContext&   appContext_;
    Session       session_;
    QTableWidget* scoreTable_ = nullptr;
    QLabel*       gpaLabel_ = nullptr;
};

} // namespace EduSys
