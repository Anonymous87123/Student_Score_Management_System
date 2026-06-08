#pragma once

#include <exception>

#include <QMainWindow>
#include <QString>

#include "EduSys/service/Session.hpp"

class QLineEdit;
class QTableWidget;

namespace EduSys {

class AppContext;
class Course;
class Student;

class AdminWindow : public QMainWindow {
public:
    AdminWindow(AppContext& appContext, Session session, QWidget* parent = nullptr);

private:
    QWidget* createStudentPage();
    QWidget* createCoursePage();
    QWidget* createScorePage();

    void refreshStudentTable();
    void showStudentById();
    void createStudent();
    void editSelectedStudent();
    void removeSelectedStudent();
    void showStudentDetails(const Student& student);
    QString selectedStudentId() const;

    void refreshCourseTable();
    void showCourseById();
    void createCourse();
    void editSelectedCourse();
    void removeSelectedCourse();
    void showCourseDetails(const Course& course);
    QString selectedCourseId() const;

    void refreshScoreTable();
    void showScoresByStudent();
    void showScoresByCourse();
    void createScore();
    void editSelectedScore();
    void removeSelectedScore();
    bool selectedScoreKey(QString& studentId, QString& courseId, QString& semester) const;

    QWidget* createStatsPage();
    void queryCourseStats();
    void queryCourseRanking();
    void queryStudentGpa();

    QWidget* createReportPage();
    void exportWarningReport();
    void exportCourseStatsCsv();
    void exportRankingCsv();

    QWidget* createAccountPage();

    void showServiceError(const QString& title, const std::exception& e);

    AppContext&   appContext_;
    Session       session_;
    QLineEdit*    studentLookupEdit_ = nullptr;
    QTableWidget* studentTable_ = nullptr;
    QLineEdit*    courseLookupEdit_ = nullptr;
    QTableWidget* courseTable_ = nullptr;
    QLineEdit*    scoreStudentLookupEdit_ = nullptr;
    QLineEdit*    scoreCourseLookupEdit_ = nullptr;
    QTableWidget* scoreTable_ = nullptr;
    QLineEdit*    statsCourseEdit_ = nullptr;
    QLineEdit*    statsStudentEdit_ = nullptr;
    QTableWidget* rankingTable_ = nullptr;
    QLineEdit*    reportCourseEdit_ = nullptr;
};

} // namespace EduSys
