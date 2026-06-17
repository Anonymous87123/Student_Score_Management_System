#include "EduSys/view/StudentMenu.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

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

std::string scoreBand(double score) {
    if (score >= 90.0) {
        return "Excellent";
    }
    if (score >= 80.0) {
        return "Good";
    }
    if (score >= 60.0) {
        return "Pass";
    }
    return "Fail";
}

std::string makeScoreBar(double score, int width = 40) {
    const double safeScore = std::clamp(score, 0.0, 100.0);
    const int filled = static_cast<int>(safeScore / 100.0 * width + 0.5);
    return std::string(filled, '#') + std::string(width - filled, '.');
}

} // namespace

void StudentMenu::printStudentScoreRows(const std::vector<Score>& list, const std::string& emptyText) {
    std::vector<std::vector<std::string>> rows;
    for (const auto& s : list) {
        rows.push_back({s.getCourseId(), s.getSemester(),
                        fmtDouble(s.getUsualScore()),
                        fmtDouble(s.getFinalScore()),
                        fmtDouble(s.getTotalScore())});
    }
    if (rows.empty()) {
        std::cout << emptyText << "\n";
        return;
    }
    printTable({"CId", "Semester", "Usual", "Final", "Total"},
               rows, {6, 12, 6, 6, 6});
}

void StudentMenu::run() {
    while (session_.isLoggedIn()) {
        printHeading("Student Menu  [" + session_.getUsername()
                     + " / studentId=" + session_.getOwnerId() + "]");
        std::cout << " 1. View my profile\n"
                  << " 2. View my courses\n"
                  << " 3. View my scores\n"
                  << " 4. View my scores by semester\n"
                  << " 5. View my scores by course\n"
                  << " 6. View my scores by course + semester\n"
                  << " 7. View my score chart\n"
                  << " 8. View my GPA\n"
                  << " 9. Change my password\n"
                  << " 0. Logout\n";
        const int choice = readInt("Select: ");
        try {
            switch (choice) {
                case 1: viewProfile();                     break;
                case 2: viewMyCourses();                   break;
                case 3: viewMyScores();                    break;
                case 4: viewMyScoresBySemester();          break;
                case 5: viewMyScoresByCourse();            break;
                case 6: viewMyScoresByCourseAndSemester(); break;
                case 7: viewScoreChart();                  break;
                case 8: viewMyGpa();                       break;
                case 9: changePassword();                  break;
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

void StudentMenu::viewMyCourses() {
    printHeading("Student > My Courses");
    auto list = scoreSvc_.findByStudent(session_, session_.getOwnerId());
    std::sort(list.begin(), list.end(), [](const Score& lhs, const Score& rhs) {
        if (lhs.getSemester() != rhs.getSemester()) {
            return lhs.getSemester() < rhs.getSemester();
        }
        return lhs.getCourseId() < rhs.getCourseId();
    });

    std::vector<std::vector<std::string>> rows;
    std::string lastKey;
    for (const auto& s : list) {
        const std::string key = s.getCourseId() + '\n' + s.getSemester();
        if (key == lastKey) {
            continue;
        }
        rows.push_back({s.getCourseId(), s.getSemester()});
        lastKey = key;
    }
    if (rows.empty()) {
        std::cout << "(no courses found from your score records)\n";
        return;
    }
    printTable({"CId", "Semester"}, rows, {6, 12});
}

void StudentMenu::viewMyScores() {
    printHeading("Student > My Scores");
    auto list = scoreSvc_.findByStudent(session_, session_.getOwnerId());
    printStudentScoreRows(list, "(no scores yet)");
}

void StudentMenu::viewMyScoresBySemester() {
    printHeading("Student > My Scores by Semester");
    const std::string semester = readLine("Semester: ");
    auto list = scoreSvc_.findByStudentAndSemester(session_, session_.getOwnerId(), semester);
    printStudentScoreRows(list, "(no scores for this semester)");
}

void StudentMenu::viewMyScoresByCourse() {
    printHeading("Student > My Scores by Course");
    const std::string cid = readLine("Course id: ");
    auto list = scoreSvc_.findByStudentAndCourse(session_, session_.getOwnerId(), cid);
    printStudentScoreRows(list, "(no scores for this course)");
}

void StudentMenu::viewMyScoresByCourseAndSemester() {
    printHeading("Student > My Scores by Course + Semester");
    const std::string cid = readLine("Course id: ");
    const std::string semester = readLine("Semester: ");
    auto list = scoreSvc_.findByStudentCourseAndSemester(session_, session_.getOwnerId(), cid, semester);
    printStudentScoreRows(list, "(no scores for this course and semester)");
}

void StudentMenu::viewScoreChart() {
    printHeading("Student > My Score Chart");
    auto list = scoreSvc_.findByStudent(session_, session_.getOwnerId());
    if (list.empty()) {
        std::cout << "(no scores yet)\n";
        return;
    }

    std::sort(list.begin(), list.end(), [](const Score& lhs, const Score& rhs) {
        if (lhs.getSemester() != rhs.getSemester()) {
            return lhs.getSemester() < rhs.getSemester();
        }
        return lhs.getCourseId() < rhs.getCourseId();
    });

    std::cout << "Scale: 0-100 total score. Each bar has 40 columns.\n"
              << "Legend: # = filled score range, . = remaining range.\n\n";

    for (const auto& s : list) {
        const std::string label = s.getCourseId() + " / " + s.getSemester();
        std::cout << std::left << std::setw(20) << label
                  << " [" << makeScoreBar(s.getTotalScore()) << "] "
                  << std::right << std::setw(6) << fmtDouble(s.getTotalScore(), 1)
                  << "  " << scoreBand(s.getTotalScore()) << "\n";
    }
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
