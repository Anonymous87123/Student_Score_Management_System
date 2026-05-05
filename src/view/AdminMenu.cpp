#include "EduSys/view/AdminMenu.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "EduSys/common/Exception.hpp"
#include "EduSys/model/Course.hpp"
#include "EduSys/model/Score.hpp"
#include "EduSys/model/Student.hpp"
#include "EduSys/report/ReportExporter.hpp"
#include "EduSys/service/AuthService.hpp"
#include "EduSys/service/CourseService.hpp"
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

std::string fmtPct(double ratio) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << (ratio * 100.0) << "%";
    return oss.str();
}

} // namespace

void AdminMenu::run() {
    while (session_.isLoggedIn()) {
        printHeading("Admin Menu  [" + session_.getUsername() + "]");
        std::cout << " 1. Student management\n"
                  << " 2. Course  management\n"
                  << " 3. Score   management\n"
                  << " 4. Statistics\n"
                  << " 5. Generate warning report\n"
                  << " 6. Change my password\n"
                  << " 7. Export course CSV (stats + ranking)\n"
                  << " 0. Logout\n";
        const int choice = readInt("Select: ");
        try {
            switch (choice) {
                case 1: studentMenu();           break;
                case 2: courseMenu();            break;
                case 3: scoreMenu();             break;
                case 4: statsMenu();             break;
                case 5: generateWarningReport(); break;
                case 6: changePassword();        break;
                case 7: exportCsv();             break;
                case 0: session_.logout();       return;
                default: printError("Unknown choice");
            }
        } catch (const EduException& e) {
            printError(e.what());
        }
    }
}

// ============================================================ Student
void AdminMenu::studentMenu() {
    while (true) {
        printHeading("Admin > Student Management");
        std::cout << " 1. List all\n"
                  << " 2. View by id\n"
                  << " 3. Add\n"
                  << " 4. Edit\n"
                  << " 5. Remove (cascade scores + user account)\n"
                  << " 0. Back\n";
        const int choice = readInt("Select: ");
        try {
            if (choice == 0) return;
            if (choice == 1) {
                auto all = studentSvc_.listAll(session_);
                std::vector<std::vector<std::string>> rows;
                for (const auto& s : all) {
                    rows.push_back({s.getId(), s.getName(), s.getClassName(),
                                    std::to_string(s.getEnrollYear()), s.getContact()});
                }
                paginate({"Id", "Name", "Class", "Year", "Contact"},
                         rows, {8, 14, 10, 6, 14});
            } else if (choice == 2) {
                const std::string id = readLine("Student id: ");
                const Student s = studentSvc_.findById(session_, id);
                std::cout << " Id      : " << s.getId() << "\n"
                          << " Name    : " << s.getName() << "\n"
                          << " Major   : " << s.getMajor() << "\n"
                          << " Class   : " << s.getClassName() << "\n"
                          << " Year    : " << s.getEnrollYear() << "\n"
                          << " Contact : " << s.getContact() << "\n";
            } else if (choice == 3) {
                const std::string id       = readLine("Id: ");
                const std::string name     = readLine("Name: ");
                const std::string contact  = readLine("Contact: ");
                const std::string major    = readLine("Major: ");
                const std::string klass    = readLine("Class: ");
                const int         year     = readInt("Enroll year: ");
                Student s(id, name, contact, major, klass, year);
                studentSvc_.create(session_, s);
                printOk("Student created: " + id);
            } else if (choice == 4) {
                const std::string id = readLine("Id to edit: ");
                Student cur = studentSvc_.findById(session_, id);
                const std::string name    = readLine("Name [" + cur.getName() + "]: ");
                const std::string contact = readLine("Contact [" + cur.getContact() + "]: ");
                const std::string major   = readLine("Major [" + cur.getMajor() + "]: ");
                const std::string klass   = readLine("Class [" + cur.getClassName() + "]: ");
                const std::string yearStr = readLine("Enroll year [" + std::to_string(cur.getEnrollYear()) + "]: ");
                Student updated(id,
                                name.empty()    ? cur.getName()      : name,
                                contact.empty() ? cur.getContact()   : contact,
                                major.empty()   ? cur.getMajor()     : major,
                                klass.empty()   ? cur.getClassName() : klass,
                                yearStr.empty() ? cur.getEnrollYear(): std::stoi(yearStr));
                studentSvc_.update(session_, updated);
                printOk("Student updated: " + id);
            } else if (choice == 5) {
                const std::string id = readLine("Id to remove: ");
                const std::string cf = readLine("Type 'yes' to confirm cascade delete: ");
                if (cf != "yes") { printError("Aborted"); continue; }
                studentSvc_.remove(session_, id);
                printOk("Student removed (cascade): " + id);
            } else {
                printError("Unknown choice");
            }
        } catch (const EduException& e) {
            printError(e.what());
        }
    }
}

// ============================================================ Course
void AdminMenu::courseMenu() {
    while (true) {
        printHeading("Admin > Course Management");
        std::cout << " 1. List all\n"
                  << " 2. View by id\n"
                  << " 3. Add\n"
                  << " 4. Edit\n"
                  << " 5. Remove (cascade scores)\n"
                  << " 0. Back\n";
        const int choice = readInt("Select: ");
        try {
            if (choice == 0) return;
            if (choice == 1) {
                auto all = courseSvc_.listAll(session_);
                std::vector<std::vector<std::string>> rows;
                for (const auto& c : all) {
                    rows.push_back({c.getCourseId(), c.getCourseName(),
                                    fmtDouble(c.getCredit(), 1),
                                    c.getTeacherId(), c.getSemester()});
                }
                paginate({"CId", "CourseName", "Cred", "TeaId", "Semester"},
                         rows, {6, 28, 5, 6, 12});
            } else if (choice == 2) {
                const std::string id = readLine("Course id: ");
                const Course c = courseSvc_.findById(session_, id);
                std::cout << " Id       : " << c.getCourseId() << "\n"
                          << " Name     : " << c.getCourseName() << "\n"
                          << " Credit   : " << c.getCredit() << "\n"
                          << " Teacher  : " << c.getTeacherId() << "\n"
                          << " Semester : " << c.getSemester() << "\n";
            } else if (choice == 3) {
                const std::string id       = readLine("Course id: ");
                const std::string name     = readLine("Course name: ");
                const double      credit   = readDouble("Credit: ");
                const std::string teacher  = readLine("Teacher id: ");
                const std::string semester = readLine("Semester: ");
                Course c(id, name, credit, teacher, semester);
                courseSvc_.create(session_, c);
                printOk("Course created: " + id);
            } else if (choice == 4) {
                const std::string id = readLine("Course id to edit: ");
                Course cur = courseSvc_.findById(session_, id);
                const std::string name    = readLine("Name [" + cur.getCourseName() + "]: ");
                const std::string credStr = readLine("Credit [" + fmtDouble(cur.getCredit(), 1) + "]: ");
                const std::string teacher = readLine("TeacherId [" + cur.getTeacherId() + "]: ");
                const std::string semester= readLine("Semester [" + cur.getSemester() + "]: ");
                Course updated(id,
                               name.empty()     ? cur.getCourseName() : name,
                               credStr.empty()  ? cur.getCredit()     : std::stod(credStr),
                               teacher.empty()  ? cur.getTeacherId()  : teacher,
                               semester.empty() ? cur.getSemester()   : semester);
                courseSvc_.update(session_, updated);
                printOk("Course updated: " + id);
            } else if (choice == 5) {
                const std::string id = readLine("Course id to remove: ");
                const std::string cf = readLine("Type 'yes' to confirm cascade delete: ");
                if (cf != "yes") { printError("Aborted"); continue; }
                courseSvc_.remove(session_, id);
                printOk("Course removed (cascade): " + id);
            } else {
                printError("Unknown choice");
            }
        } catch (const EduException& e) {
            printError(e.what());
        }
    }
}

// ============================================================ Score
void AdminMenu::scoreMenu() {
    while (true) {
        printHeading("Admin > Score Management");
        std::cout << " 1. List all\n"
                  << " 2. By student\n"
                  << " 3. By course\n"
                  << " 4. Upsert (add or update)\n"
                  << " 5. Remove one score\n"
                  << " 0. Back\n";
        const int choice = readInt("Select: ");
        try {
            if (choice == 0) return;
            if (choice == 1) {
                auto all = scoreSvc_.listAll(session_);
                std::vector<std::vector<std::string>> rows;
                for (const auto& s : all) {
                    rows.push_back({s.getStudentId(), s.getCourseId(), s.getSemester(),
                                    fmtDouble(s.getUsualScore()),
                                    fmtDouble(s.getFinalScore()),
                                    fmtDouble(s.getTotalScore())});
                }
                paginate({"StuId", "CId", "Semester", "Usual", "Final", "Total"},
                         rows, {6, 6, 12, 6, 6, 6});
            } else if (choice == 2) {
                const std::string sid = readLine("Student id: ");
                auto list = scoreSvc_.findByStudent(session_, sid);
                std::vector<std::vector<std::string>> rows;
                for (const auto& s : list) {
                    rows.push_back({s.getCourseId(), s.getSemester(),
                                    fmtDouble(s.getUsualScore()),
                                    fmtDouble(s.getFinalScore()),
                                    fmtDouble(s.getTotalScore())});
                }
                printTable({"CId", "Semester", "Usual", "Final", "Total"},
                           rows, {6, 12, 6, 6, 6});
            } else if (choice == 3) {
                const std::string cid = readLine("Course id: ");
                auto list = scoreSvc_.findByCourse(session_, cid);
                std::vector<std::vector<std::string>> rows;
                for (const auto& s : list) {
                    rows.push_back({s.getStudentId(), s.getSemester(),
                                    fmtDouble(s.getUsualScore()),
                                    fmtDouble(s.getFinalScore()),
                                    fmtDouble(s.getTotalScore())});
                }
                printTable({"StuId", "Semester", "Usual", "Final", "Total"},
                           rows, {6, 12, 6, 6, 6});
            } else if (choice == 4) {
                const std::string sid = readLine("Student id: ");
                const std::string cid = readLine("Course id: ");
                const std::string sem = readLine("Semester: ");
                const double usual = readDouble("Usual (0-100): ");
                const double final = readDouble("Final (0-100): ");
                const double total = readDouble("Total (0-100): ");
                Score s(sid, cid, sem, usual, final, total);
                scoreSvc_.upsert(session_, s);
                printOk("Score upserted: " + sid + "/" + cid + "/" + sem);
            } else if (choice == 5) {
                const std::string sid = readLine("Student id: ");
                const std::string cid = readLine("Course id: ");
                const std::string sem = readLine("Semester: ");
                scoreSvc_.remove(session_, sid, cid, sem);
                printOk("Score removed: " + sid + "/" + cid + "/" + sem);
            } else {
                printError("Unknown choice");
            }
        } catch (const EduException& e) {
            printError(e.what());
        }
    }
}

// ============================================================ Stats
void AdminMenu::statsMenu() {
    while (true) {
        printHeading("Admin > Statistics");
        std::cout << " 1. Course stats (count/avg/max/min/pass/excellent)\n"
                  << " 2. Ranking by course (total score desc)\n"
                  << " 3. Student GPA\n"
                  << " 0. Back\n";
        const int choice = readInt("Select: ");
        try {
            if (choice == 0) return;
            if (choice == 1) {
                const std::string cid = readLine("Course id: ");
                const CourseStats st = statsSvc_.computeCourseStats(session_, cid);
                std::cout << " CourseId    : " << st.courseId    << "\n"
                          << " CourseName  : " << st.courseName  << "\n"
                          << " Count       : " << st.count       << "\n"
                          << " Avg         : " << fmtDouble(st.avg) << "\n"
                          << " Max         : " << fmtDouble(st.max) << "\n"
                          << " Min         : " << fmtDouble(st.min) << "\n"
                          << " Pass rate   : " << fmtPct(st.passRate)      << "\n"
                          << " Excellent   : " << fmtPct(st.excellentRate) << "\n";
            } else if (choice == 2) {
                const std::string cid = readLine("Course id: ");
                const auto list = statsSvc_.rankByCourse(session_, cid);
                std::vector<std::vector<std::string>> rows;
                int rank = 1;
                for (const auto& e : list) {
                    rows.push_back({std::to_string(rank++), e.studentId, e.studentName,
                                    fmtDouble(e.totalScore)});
                }
                printTable({"Rank", "StuId", "Name", "Total"},
                           rows, {5, 8, 14, 7});
            } else if (choice == 3) {
                const std::string sid = readLine("Student id: ");
                const GpaResult g = statsSvc_.computeGpaFor(session_, sid);
                std::cout << " StudentId   : " << g.studentId   << "\n"
                          << " StudentName : " << g.studentName << "\n"
                          << " Courses     : " << g.courseCount << "\n"
                          << " Total credit: " << fmtDouble(g.totalCredit, 1) << "\n"
                          << " GPA         : " << fmtDouble(g.gpa) << " / 4.0\n";
            } else {
                printError("Unknown choice");
            }
        } catch (const EduException& e) {
            printError(e.what());
        }
    }
}

// ============================================================ Warning report
void AdminMenu::generateWarningReport() {
    printHeading("Admin > Warning Report");
    const std::string path = reportExporter_.exportWarningReport(session_);
    printOk("Warning report written to: " + path);
}

// ============================================================ CSV export (Week 14)
// Admin 专用：选一个 courseId，一次性导出该课程的统计 CSV 与排名 CSV。
// 不改任何统计口径 / 主菜单流程；ReportExporter 内部走相同的 StatsService 接口。
void AdminMenu::exportCsv() {
    printHeading("Admin > Export Course CSV");
    const std::string courseId = readLine("Course id: ");
    const std::string statsPath = reportExporter_.exportCourseStatsCsv(session_, courseId);
    printOk("Course stats CSV : " + statsPath);
    const std::string rankPath  = reportExporter_.exportRankingCsv(session_, courseId);
    printOk("Ranking CSV      : " + rankPath);
}

// ============================================================ Password
void AdminMenu::changePassword() {
    printHeading("Admin > Change Password");
    const std::string oldPw = readLine("Old password: ");
    const std::string newPw = readLine("New password: ");
    authSvc_.changePassword(session_, oldPw, newPw);
    printOk("Password changed.");
}

} // namespace EduSys
