#pragma once

#include <exception>
#include <vector>

#include <QMainWindow>
#include <QString>

#include "EduSys/model/Course.hpp"
#include "EduSys/service/Session.hpp"

class QComboBox;
class QLineEdit;
class QTableWidget;

namespace EduSys {

class AppContext;

class TeacherWindow : public QMainWindow {
public:
    TeacherWindow(AppContext& appContext, Session session, QWidget* parent = nullptr);

private:
    QWidget* createMyCoursesPage();
    QWidget* createMyScoresPage();
    QWidget* createMyStatsPage();
    QWidget* createAccountPage();

    void refreshMyCourses();
    void refreshMyScores();
    void createScore();
    void editSelectedScore();
    void removeSelectedScore();
    bool selectedScoreKey(QString& studentId, QString& courseId, QString& semester) const;

    void queryCourseStats();
    void queryCourseRanking();

    void showServiceError(const QString& title, const std::exception& e);

    std::vector<Course> myCourses();

    AppContext&   appContext_;
    Session       session_;
    QTableWidget* courseTable_ = nullptr;
    QComboBox*    scoreCourseCombo_ = nullptr;
    QTableWidget* scoreTable_ = nullptr;
    QComboBox*    statsCourseCombo_ = nullptr;
    QTableWidget* rankingTable_ = nullptr;
};

} // namespace EduSys
