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

void TeacherMenu::run() {
    while (session_.isLoggedIn()) {
        printHeading("Teacher Menu  [" + session_.getUsername()
                     + " / teacherId=" + session_.getOwnerId() + "]");
        std::cout << " 1. List my courses\n"
                  << " 2. View scores for my course\n"
                  << " 3. Record / update one score\n"
                  << " 4. Delete one score\n"
                  << " 5. Course statistics (my course only)\n"
                  << " 6. Change my password\n"
                  << " 0. Logout\n";
        const int choice = readInt("Select: ");
        try {
            switch (choice) {
                case 1: listMyCourses();   break;
                case 2: viewMyScores();    break;
                case 3: upsertScore();     break;
                case 4: deleteScore();     break;
                case 5: courseStats();     break;
                case 6: changePassword();  break;
                case 0: session_.logout(); return;
                default: printError("Unknown choice");
            }
        } catch (const EduException& e) {
            printError(e.what());
        }
    }
}

// 严格过滤：teacherId == session.ownerId 的那部分课程才允许出现在所有 Teacher 菜单上下文中。
std::string TeacherMenu::pickOwnCourseId() {
    auto all = courseSvc_.listAll(session_);
    std::vector<Course> mine;
    std::copy_if(all.begin(), all.end(), std::back_inserter(mine),
        [&](const Course& c) { return c.getTeacherId() == session_.getOwnerId(); });

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
        if (c.getTeacherId() != session_.getOwnerId()) continue;
        rows.push_back({c.getCourseId(), c.getCourseName(),
                        fmtDouble(c.getCredit(), 1), c.getSemester()});
    }
    if (rows.empty()) {
        std::cout << "(no courses assigned to you)\n";
        return;
    }
    printTable({"CId", "CourseName", "Cred", "Semester"}, rows, {6, 28, 5, 12});
}

void TeacherMenu::viewMyScores() {
    printHeading("Teacher > Scores for My Course");
    const std::string cid = pickOwnCourseId();
    if (cid.empty()) return;

    auto list = scoreSvc_.findByCourse(session_, cid);
    std::vector<std::vector<std::string>> rows;
    for (const auto& s : list) {
        rows.push_back({s.getStudentId(), s.getSemester(),
                        fmtDouble(s.getUsualScore()),
                        fmtDouble(s.getFinalScore()),
                        fmtDouble(s.getTotalScore())});
    }
    if (rows.empty()) {
        std::cout << "(no scores for this course)\n";
        return;
    }
    printTable({"StuId", "Semester", "Usual", "Final", "Total"},
               rows, {8, 12, 6, 6, 6});
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
