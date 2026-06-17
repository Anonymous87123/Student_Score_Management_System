#pragma once

#include <exception>
#include <vector>

#include <QMainWindow>
#include <QString>

#include "EduSys/service/Session.hpp"

class QLineEdit;
class QTableWidget;

namespace EduSys {

class AppContext;
class BackgroundHostWidget;
class Course;
class Student;
class Teacher;

class AdminWindow : public QMainWindow {
public:
    AdminWindow(AppContext& appContext, Session session, QWidget* parent = nullptr);

private:
    QWidget* createStudentPage();
    QWidget* createTeacherPage();
    QWidget* createCoursePage();
    QWidget* createScorePage();

    void refreshStudentTable();
    void showStudentsSortedById();
    void showStudentsSortedByName();
    void showStudentById();
    void createStudent();
    void editSelectedStudent();
    void removeSelectedStudent();
    void showStudentDetails(const Student& student);
    QString selectedStudentId() const;

    void refreshTeacherTable();
    void showTeacherById();
    void createTeacher();
    void editSelectedTeacher();
    void removeSelectedTeacher();
    void showTeacherDetails(const Teacher& teacher);
    QString selectedTeacherId() const;

    void refreshCourseTable();
    void showCourseById();
    void showCoursesBySemester();
    void showCoursesByTeacher();
    void showCoursesByTeacherAndSemester();
    void createCourse();
    void editSelectedCourse();
    void removeSelectedCourse();
    void showCourseDetails(const Course& course);
    QString selectedCourseId() const;

    void refreshScoreTable();
    void showScoresByStudent();
    void showScoresByCourse();
    void showScoresByClass();
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
    QWidget* createBackgroundSettingsPage();
    BackgroundHostWidget* createBackgroundPage();
    void refreshBackgroundPages();
    void chooseBackgroundImage();
    void clearBackgroundImage();
    void updateBackgroundOpacity(int value);

    void showServiceError(const QString& title, const std::exception& e);

    AppContext&   appContext_;
    Session       session_;
    std::vector<Student> studentRows_;
    QLineEdit*    studentLookupEdit_ = nullptr;
    QTableWidget* studentTable_ = nullptr;
    QLineEdit*    teacherLookupEdit_ = nullptr;
    QTableWidget* teacherTable_ = nullptr;
    QLineEdit*    courseLookupEdit_ = nullptr;
    QLineEdit*    courseSemesterLookupEdit_ = nullptr;
    QLineEdit*    courseTeacherLookupEdit_ = nullptr;
    QTableWidget* courseTable_ = nullptr;
    QLineEdit*    scoreStudentLookupEdit_ = nullptr;
    QLineEdit*    scoreCourseLookupEdit_ = nullptr;
    QLineEdit*    scoreClassLookupEdit_ = nullptr;
    QTableWidget* scoreTable_ = nullptr;
    QLineEdit*    statsCourseEdit_ = nullptr;
    QLineEdit*    statsStudentEdit_ = nullptr;
    QTableWidget* rankingTable_ = nullptr;
    QLineEdit*    reportCourseEdit_ = nullptr;
    std::vector<BackgroundHostWidget*> backgroundPages_;
};

} // namespace EduSys
