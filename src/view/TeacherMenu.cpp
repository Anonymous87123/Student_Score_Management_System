#include "EduSys/view/TeacherMenu.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "EduSys/common/Exception.hpp"
#include "EduSys/model/Course.hpp"
#include "EduSys/model/Score.hpp"
#include "EduSys/service/AuthService.hpp"
#include "EduSys/service/CourseService.hpp"
#include "EduSys/service/ScoreService.hpp"
#include "EduSys/service/Session.hpp"
#include "EduSys/service/StatsService.hpp"

namespace EduSys {

namespace {

std::string fmtDouble(double v, int prec = 2) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(prec) << v;
    return oss.str();
}

std::string fmtPct(double ratio) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << (ratio * 100.0) << "%";
    return oss.str();
}

} // namespace

void TeacherMenu::printScoreRows(const std::vector<Score>& list, const std::string& emptyText) {
    std::vector<std::vector<std::string>> rows;
    for (const auto& s : list) {
        rows.push_back({s.getStudentId(), s.getCourseId(), s.getSemester(),
                        fmtDouble(s.getUsualScore()),
                        fmtDouble(s.getFinalScore()),
                        fmtDouble(s.getTotalScore())});
    }
    if (rows.empty()) {
        std::cout << emptyText << "\n";
        return;
    }
    paginate({"StuId", "CId", "Semester", "Usual", "Final", "Total"},
             rows, {8, 6, 12, 6, 6, 6});
}

void TeacherMenu::run() {
    while (session_.isLoggedIn()) {
        printHeading("Teacher Menu  [" + session_.getUsername()
                     + " / teacherId=" + session_.getOwnerId() + "]");
        std::cout << " 1. List my courses\n"
                  << " 2. List my courses by semester\n"
                  << " 3. View scores for my course\n"
                  << " 4. View scores by student\n"
                  << " 5. View scores by class\n"
                  << " 6. View scores by course + semester\n"
                  << " 7. Course ranking\n"
                  << " 8. Record / update one score\n"
                  << " 9. Delete one score\n"
                  << "10. Course statistics (my course only)\n"
                  << "11. Change my password\n"
                  << " 0. Logout\n";
        const int choice = readInt("Select: ");
        try {
            switch (choice) {
                case 1:  listMyCourses();                 break;
                case 2:  listMyCoursesBySemester();       break;
                case 3:  viewMyScores();                  break;
                case 4:  viewScoresByStudent();           break;
                case 5:  viewScoresByClass();             break;
                case 6:  viewScoresByCourseAndSemester(); break;
                case 7:  courseRanking();                 break;
                case 8:  upsertScore();                   break;
                case 9:  deleteScore();                   break;
                case 10: courseStats();                   break;
                case 11: changePassword();                break;
                case 0: session_.logout(); return;
                default: printError("Unknown choice");
            }
        } catch (const EduException& e) {
            printError(e.what());
        }
    }
}

// 严格过滤：授课教师列表包含 session.ownerId 的课程才允许出现在所有 Teacher 菜单上下文中。
std::string TeacherMenu::pickOwnCourseId() {
    auto all = courseSvc_.listAll(session_);
    std::vector<Course> mine;
    std::copy_if(all.begin(), all.end(), std::back_inserter(mine),
        [&](const Course& c) { return c.hasTeacher(session_.getOwnerId()); });

    if (mine.empty()) {
        printError("You have no courses assigned.");
        return {};
    }

    std::vector<std::vector<std::string>> rows;
    for (const auto& c : mine) {
        rows.push_back({c.getCourseId(), c.getCourseName(),
                        fmtDouble(c.getCredit(), 1), c.getSemester()});
    }
    printTable({"CId", "CourseName", "Cred", "Semester"}, rows, {6, 28, 5, 12});

    const std::string cid = readLine("Pick course id from above (empty = cancel): ");
    if (cid.empty()) return {};

    auto it = std::find_if(mine.begin(), mine.end(),
        [&](const Course& c) { return c.getCourseId() == cid; });
    if (it == mine.end()) {
        printError("Course id is not in your teaching list: " + cid);
        return {};
    }
    return cid;
}

void TeacherMenu::listMyCourses() {
    printHeading("Teacher > My Courses");
    auto all = courseSvc_.listAll(session_);
    std::vector<std::vector<std::string>> rows;
    for (const auto& c : all) {
        if (!c.hasTeacher(session_.getOwnerId())) continue;
        rows.push_back({c.getCourseId(), c.getCourseName(),
                        fmtDouble(c.getCredit(), 1), c.getSemester()});
    }
    if (rows.empty()) {
        std::cout << "(no courses assigned to you)\n";
        return;
    }
    printTable({"CId", "CourseName", "Cred", "Semester"}, rows, {6, 28, 5, 12});
}

void TeacherMenu::listMyCoursesBySemester() {
    printHeading("Teacher > My Courses by Semester");
    const std::string semester = readLine("Semester: ");
    auto list = courseSvc_.listByTeacherAndSemester(session_, session_.getOwnerId(), semester);

    std::vector<std::vector<std::string>> rows;
    for (const auto& c : list) {
        rows.push_back({c.getCourseId(), c.getCourseName(),
                        fmtDouble(c.getCredit(), 1), c.getSemester()});
    }
    if (rows.empty()) {
        std::cout << "(no courses assigned to you in this semester)\n";
        return;
    }
    printTable({"CId", "CourseName", "Cred", "Semester"}, rows, {6, 28, 5, 12});
}

void TeacherMenu::viewMyScores() {
    printHeading("Teacher > Scores for My Course");
    const std::string cid = pickOwnCourseId();
    if (cid.empty()) return;

    auto list = scoreSvc_.findByCourse(session_, cid);
    printScoreRows(list, "(no scores for this course)");
}

void TeacherMenu::viewScoresByStudent() {
    printHeading("Teacher > Scores by Student");
    const std::string sid = readLine("Student id: ");
    auto list = scoreSvc_.findByStudent(session_, sid);
    printScoreRows(list, "(no scores for this student in your courses)");
}

void TeacherMenu::viewScoresByClass() {
    printHeading("Teacher > Scores by Class");
    const std::string klass = readLine("Class: ");
    auto list = scoreSvc_.findByClass(session_, klass);
    printScoreRows(list, "(no scores for this class in your courses)");
}

void TeacherMenu::viewScoresByCourseAndSemester() {
    printHeading("Teacher > Scores by Course + Semester");
    const std::string cid = pickOwnCourseId();
    if (cid.empty()) return;

    const std::string semester = readLine("Semester: ");
    auto list = scoreSvc_.findByCourseAndSemester(session_, cid, semester);
    printScoreRows(list, "(no scores for this course and semester)");
}

void TeacherMenu::courseRanking() {
    printHeading("Teacher > Course Ranking");
    const std::string cid = pickOwnCourseId();
    if (cid.empty()) return;

    const auto list = statsSvc_.rankByCourse(session_, cid);
    std::vector<std::vector<std::string>> rows;
    int rank = 1;
    for (const auto& e : list) {
        rows.push_back({std::to_string(rank++), e.studentId, e.studentName,
                        fmtDouble(e.totalScore)});
    }
    if (rows.empty()) {
        std::cout << "(no ranking data for this course)\n";
        return;
    }
    printTable({"Rank", "StuId", "Name", "Total"}, rows, {5, 8, 14, 7});
}

void TeacherMenu::upsertScore() {
    printHeading("Teacher > Record / Update Score");
    const std::string cid = pickOwnCourseId();
    if (cid.empty()) return;

    const std::string sid = readLine("Student id: ");
    const std::string sem = readLine("Semester: ");
    const double usual = readDouble("Usual (0-100): ");
    const double final = readDouble("Final (0-100): ");
    const double total = readDouble("Total (0-100): ");
    Score s(sid, cid, sem, usual, final, total);
    scoreSvc_.upsert(session_, s);
    printOk("Score upserted: " + sid + "/" + cid + "/" + sem);
}

void TeacherMenu::deleteScore() {
    printHeading("Teacher > Delete Score");
    const std::string cid = pickOwnCourseId();
    if (cid.empty()) return;

    const std::string sid = readLine("Student id: ");
    const std::string sem = readLine("Semester: ");
    scoreSvc_.remove(session_, sid, cid, sem);
    printOk("Score removed: " + sid + "/" + cid + "/" + sem);
}

void TeacherMenu::courseStats() {
    printHeading("Teacher > Course Statistics");
    const std::string cid = pickOwnCourseId();
    if (cid.empty()) return;

    const CourseStats st = statsSvc_.computeCourseStats(session_, cid);
    std::cout << " CourseId    : " << st.courseId    << "\n"
              << " CourseName  : " << st.courseName  << "\n"
              << " Count       : " << st.count       << "\n"
              << " Avg         : " << fmtDouble(st.avg) << "\n"
              << " Max         : " << fmtDouble(st.max) << "\n"
              << " Min         : " << fmtDouble(st.min) << "\n"
              << " Pass rate   : " << fmtPct(st.passRate)      << "\n"
              << " Excellent   : " << fmtPct(st.excellentRate) << "\n";
}

void TeacherMenu::changePassword() {
    printHeading("Teacher > Change Password");
    const std::string oldPw = readLine("Old password: ");
    const std::string newPw = readLine("New password: ");
    authSvc_.changePassword(session_, oldPw, newPw);
    printOk("Password changed.");
}

} // namespace EduSys
