#pragma once

#include <exception>
#include <string>
#include <unordered_map>
#include <vector>

#include <QMainWindow>
#include <QString>

#include "EduSys/model/Course.hpp"
#include "EduSys/model/Score.hpp"
#include "EduSys/service/Session.hpp"

class QComboBox;
class QLineEdit;
class QTableWidget;

namespace EduSys {

class AppContext;
class BackgroundHostWidget;

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
    void showScoresSortedByStudentId();
    void showScoresSortedByStudentName();
    void showScoresSortedByTotalDesc();
    void createScore();
    void editSelectedScore();
    void removeSelectedScore();
    bool selectedScoreKey(QString& studentId, QString& courseId, QString& semester) const;
    void populateScoreTable();
    QString studentNameFor(const std::string& studentId) const;

    void queryCourseStats();
    void queryCourseRanking();

    QWidget* createBackgroundSettingsPage();
    BackgroundHostWidget* createBackgroundPage();
    void refreshBackgroundPages();
    void chooseBackgroundImage();
    void clearBackgroundImage();
    void updateBackgroundOpacity(int value);

    void showServiceError(const QString& title, const std::exception& e);

    std::vector<Course> myCourses();

    AppContext&   appContext_;
    Session       session_;
    QTableWidget* courseTable_ = nullptr;
    QLineEdit*    courseSemesterFilter_ = nullptr;
    QComboBox*    scoreCourseCombo_ = nullptr;
    QLineEdit*    scoreStudentFilter_ = nullptr;
    QLineEdit*    scoreClassFilter_ = nullptr;
    QLineEdit*    scoreSemesterFilter_ = nullptr;
    QTableWidget* scoreTable_ = nullptr;
    std::vector<Score> scoreRows_;
    std::unordered_map<std::string, std::string> scoreStudentNames_;
    QComboBox*    statsCourseCombo_ = nullptr;
    QTableWidget* rankingTable_ = nullptr;
    std::vector<BackgroundHostWidget*> backgroundPages_;
};

} // namespace EduSys
