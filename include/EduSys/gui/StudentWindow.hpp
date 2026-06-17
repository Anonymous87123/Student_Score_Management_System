#pragma once

#include <exception>
#include <string>
#include <vector>

#include <QMainWindow>
#include <QString>

#include "EduSys/model/Score.hpp"
#include "EduSys/service/Session.hpp"

class QLabel;
class QLineEdit;
class QTableWidget;

namespace EduSys {

class AppContext;
class BackgroundHostWidget;
class ScoreBarChartWidget;

class StudentWindow : public QMainWindow {
public:
    StudentWindow(AppContext& appContext, Session session, QWidget* parent = nullptr);

private:
    QWidget* createProfilePage();
    QWidget* createMyCoursesPage();
    QWidget* createMyScoresPage();
    QWidget* createScoreChartPage();
    QWidget* createMyGpaPage();
    QWidget* createAccountPage();
    QWidget* createBackgroundSettingsPage();

    void refreshMyCourses();
    void refreshMyScores();
    void refreshScoreChart();
    void refreshMyGpa();
    BackgroundHostWidget* createBackgroundPage();
    void refreshBackgroundPages();
    void chooseBackgroundImage();
    void clearBackgroundImage();
    void updateBackgroundOpacity(int value);

    void showServiceError(const QString& title, const std::exception& e);

    AppContext&   appContext_;
    Session       session_;
    QTableWidget* courseTable_ = nullptr;
    QLineEdit*    courseSemesterFilter_ = nullptr;
    QLineEdit*    scoreCourseFilter_ = nullptr;
    QLineEdit*    scoreSemesterFilter_ = nullptr;
    QTableWidget* scoreTable_ = nullptr;
    std::vector<Score> scoreRows_;
    ScoreBarChartWidget* scoreChart_ = nullptr;
    QLabel*       gpaLabel_ = nullptr;
    std::vector<BackgroundHostWidget*> backgroundPages_;
};

} // namespace EduSys
