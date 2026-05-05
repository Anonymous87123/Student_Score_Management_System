#include "EduSys/view/StudentMenu.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "EduSys/common/Exception.hpp"
#include "EduSys/model/Score.hpp"
#include "EduSys/model/Student.hpp"
#include "EduSys/service/AuthService.hpp"
#include "EduSys/service/ScoreService.hpp"
#include "EduSys/service/Session.hpp"
#include "EduSys/service/StatsService.hpp"
#include "EduSys/service/StudentService.hpp"

namespace EduSys {

namespace {

std::string fmtDouble(double v, int prec = 2) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(prec) << v;
    return oss.str();
}

} // namespace

void StudentMenu::run() {
    while (session_.isLoggedIn()) {
        printHeading("Student Menu  [" + session_.getUsername()
                     + " / studentId=" + session_.getOwnerId() + "]");
        std::cout << " 1. View my profile\n"
                  << " 2. View my scores\n"
                  << " 3. View my GPA\n"
                  << " 4. Change my password\n"
                  << " 0. Logout\n";
        const int choice = readInt("Select: ");
        try {
            switch (choice) {
                case 1: viewProfile();     break;
                case 2: viewMyScores();    break;
                case 3: viewMyGpa();       break;
                case 4: changePassword();  break;
                case 0: session_.logout(); return;
                default: printError("Unknown choice");
            }
        } catch (const EduException& e) {
            printError(e.what());
        }
    }
}

void StudentMenu::viewProfile() {
    printHeading("Student > My Profile");
    const Student s = studentSvc_.findById(session_, session_.getOwnerId());
    std::cout << " Id      : " << s.getId() << "\n"
              << " Name    : " << s.getName() << "\n"
              << " Major   : " << s.getMajor() << "\n"
              << " Class   : " << s.getClassName() << "\n"
              << " Year    : " << s.getEnrollYear() << "\n"
              << " Contact : " << s.getContact() << "\n";
}

void StudentMenu::viewMyScores() {
    printHeading("Student > My Scores");
    auto list = scoreSvc_.findByStudent(session_, session_.getOwnerId());
    std::vector<std::vector<std::string>> rows;
    for (const auto& s : list) {
        rows.push_back({s.getCourseId(), s.getSemester(),
                        fmtDouble(s.getUsualScore()),
                        fmtDouble(s.getFinalScore()),
                        fmtDouble(s.getTotalScore())});
    }
    if (rows.empty()) {
        std::cout << "(no scores yet)\n";
        return;
    }
    printTable({"CId", "Semester", "Usual", "Final", "Total"},
               rows, {6, 12, 6, 6, 6});
}

void StudentMenu::viewMyGpa() {
    printHeading("Student > My GPA");
    const GpaResult g = statsSvc_.computeGpaFor(session_, session_.getOwnerId());
    std::cout << " StudentId   : " << g.studentId   << "\n"
              << " StudentName : " << g.studentName << "\n"
              << " Courses     : " << g.courseCount << "\n"
              << " Total credit: " << fmtDouble(g.totalCredit, 1) << "\n"
              << " GPA         : " << fmtDouble(g.gpa) << " / 4.0\n";
}

void StudentMenu::changePassword() {
    printHeading("Student > Change Password");
    const std::string oldPw = readLine("Old password: ");
    const std::string newPw = readLine("New password: ");
    authSvc_.changePassword(session_, oldPw, newPw);
    printOk("Password changed.");
}

} // namespace EduSys
