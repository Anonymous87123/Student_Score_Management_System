#include <algorithm>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "EduSys/app/AppContext.hpp"
#include "EduSys/common/Exception.hpp"
#include "EduSys/common/Logger.hpp"
#include "EduSys/common/Types.hpp"
#include "EduSys/model/Course.hpp"
#include "EduSys/model/Score.hpp"
#include "EduSys/model/Student.hpp"
#include "EduSys/model/UserAccount.hpp"
#include "EduSys/service/AuthService.hpp"
#include "EduSys/service/CourseService.hpp"
#include "EduSys/service/ScoreService.hpp"
#include "EduSys/service/Session.hpp"
#include "EduSys/service/StudentService.hpp"
#include "EduSys/view/AdminMenu.hpp"
#include "EduSys/view/StudentMenu.hpp"
#include "EduSys/view/TeacherMenu.hpp"

namespace {

const std::string kSeedAdminPlain   = "admin123";

void runWeek11SelfCheck(EduSys::StudentRepository& studentRepo,
                        EduSys::TeacherRepository& teacherRepo,
                        EduSys::UserRepository&    userRepo,
                        EduSys::CourseRepository&  courseRepo,
                        EduSys::ScoreRepository&   scoreRepo,
                        bool seededThisRun) {
    using namespace EduSys;

    auto& logger = Logger::instance();
    AuthService    authSvc(userRepo);
    StudentService studentSvc(studentRepo, scoreRepo, userRepo);
    CourseService  courseSvc(courseRepo, scoreRepo, teacherRepo);
    ScoreService   scoreSvc(scoreRepo, courseRepo, studentRepo);

    UserAccount adminAcc = authSvc.authenticate("admin", kSeedAdminPlain);
    std::cout << " [Week11] AUTH OK            : admin (role=Admin)\n";

    Session adminSession;
    adminSession.login(adminAcc.getUsername(), adminAcc.getRole(), adminAcc.getOwnerId());

    bool badPwRejected = false;
    try {
        authSvc.authenticate("admin", "wrong-password");
    } catch (const AuthException& e) {
        badPwRejected = true;
        std::cout << " [Week11] AUTH FAIL expected : " << e.what() << "\n";
    }
    if (!badPwRejected) {
        throw EduException("Self-check: bad password was NOT rejected");
    }

    Session studentSession;
    studentSession.login("s001", RoleType::Student, "S001");
    bool studentCreateRejected = false;
    try {
        Student fake("S999", "Should Fail", "0", "X", "Y", 2025);
        studentSvc.create(studentSession, fake);
    } catch (const PermissionException&) {
        studentCreateRejected = true;
    }
    if (!studentCreateRejected) {
        throw EduException("Self-check: student-role create() was NOT rejected");
    }
    std::cout << " [Week11] PERM REJECT ok     : student cannot create student\n";

    bool rangeRejected = false;
    try {
        Score bad("S001", "C001", "2025-2026-1", 999.0, 50.0, 60.0);
        scoreSvc.upsert(adminSession, bad);
    } catch (const ValidationException&) {
        rangeRejected = true;
    }
    if (!rangeRejected) {
        throw EduException("Self-check: out-of-range score was NOT rejected");
    }
    std::cout << " [Week11] RANGE REJECT ok    : usual=999 rejected\n";

    Session teacherSession;
    teacherSession.login("t001", RoleType::Teacher, "T001");

    if (seededThisRun) {
        Student s003("S003", "Wang Wu", "13800000003", "Computer Science", "CS2501", 2025);
        studentSvc.create(adminSession, s003);

        Score s003Score("S003", "C001", "2025-2026-1", 92.0, 95.0, 94.1);
        scoreSvc.upsert(teacherSession, s003Score);

        Score s001Updated("S001", "C001", "2025-2026-1", 88.0, 92.0, 90.8);
        scoreSvc.upsert(adminSession, s001Updated);

        studentSvc.remove(adminSession, "S002");

        logger.info("Week 11 mutation block executed (S003 created, S001 score updated, S002 cascade-deleted).");
        std::cout << " [Week11] MUTATIONS done     : +S003, S001 score updated, -S002 (cascade)\n";
    } else {
        std::cout << " [Week11] MUTATIONS skipped  : LOADED run, verifying persisted state instead\n";
    }

    auto finalStudents = studentRepo.loadAll();
    auto finalUsers    = userRepo.loadAll();
    auto finalScores   = scoreRepo.loadAll();

    auto hasStudent = [&](const std::string& id) {
        return std::any_of(finalStudents.begin(), finalStudents.end(),
            [&](const Student& s) { return s.getId() == id; });
    };
    auto hasUserOwner = [&](const std::string& owner) {
        return std::any_of(finalUsers.begin(), finalUsers.end(),
            [&](const UserAccount& u) {
                return u.getRole() == RoleType::Student && u.getOwnerId() == owner;
            });
    };
    auto hasScoreFor = [&](const std::string& sid) {
        return std::any_of(finalScores.begin(), finalScores.end(),
            [&](const Score& s) { return s.getStudentId() == sid; });
    };

    if (hasStudent("S002")) {
        throw EduException("Self-check: S002 should be deleted");
    }
    if (hasUserOwner("S002")) {
        throw EduException("Self-check: s002 user account should be deleted");
    }
    if (hasScoreFor("S002")) {
        throw EduException("Self-check: scores for S002 should be cascade-deleted");
    }
    if (!hasStudent("S003")) {
        throw EduException("Self-check: S003 should exist");
    }
    if (!hasScoreFor("S003")) {
        throw EduException("Self-check: score for S003 should exist");
    }

    std::cout << " [Week11] FINAL ASSERT ok    : S002 absent, S003 present, scores consistent\n";
    logger.info("Week 11 self-check PASSED.");
}

template <typename Exc, typename F>
void assertThrowsWeek13(const std::string& label, F&& fn) {
    try {
        fn();
    } catch (const Exc&) {
        std::cout << "   [" << label << "] PASS\n";
        return;
    } catch (const std::exception& e) {
        throw EduSys::EduException("[" + label + "] unexpected exception: " + e.what());
    }
    throw EduSys::EduException("[" + label + "] did not throw expected exception");
}

void runWeek13BoundaryCheck(EduSys::StudentRepository& studentRepo,
                            EduSys::TeacherRepository& teacherRepo,
                            EduSys::UserRepository&    userRepo,
                            EduSys::CourseRepository&  courseRepo,
                            EduSys::ScoreRepository&   scoreRepo) {
    using namespace EduSys;

    AuthService    authSvc(userRepo);
    StudentService studentSvc(studentRepo, scoreRepo, userRepo);
    CourseService  courseSvc(courseRepo, scoreRepo, teacherRepo);
    ScoreService   scoreSvc(scoreRepo, courseRepo, studentRepo);

    Session adminSession;
    adminSession.login("admin", RoleType::Admin, "");

    Session studentSession;
    studentSession.login("s001", RoleType::Student, "S001");

    Session teacherT001;
    teacherT001.login("t001", RoleType::Teacher, "T001");

    Session teacherGhost;
    teacherGhost.login("ghost", RoleType::Teacher, "T999");

    Session emptySession;

    std::cout << "\n -- Week 13 boundary checks (A-E groups, read-only) --\n";

    std::cout << "  Group A (auth / session):\n";
    assertThrowsWeek13<AuthException>("A1 unknown user",
        [&]{ authSvc.authenticate("no_such_user", "x"); });
    assertThrowsWeek13<AuthException>("A2 bad password",
        [&]{ authSvc.authenticate("admin", "wrong-password"); });
    assertThrowsWeek13<AuthException>("A3 changePassword while unlogged",
        [&]{ authSvc.changePassword(emptySession, "x", "y"); });
    assertThrowsWeek13<ValidationException>("A4 changePassword empty new",
        [&]{ authSvc.changePassword(adminSession, kSeedAdminPlain, ""); });
    assertThrowsWeek13<AuthException>("A5 unlogged read",
        [&]{ studentSvc.listAll(emptySession); });

    std::cout << "  Group B (field validation):\n";
    assertThrowsWeek13<ValidationException>("B1 score usual=101",
        [&]{ scoreSvc.upsert(adminSession, Score("S001", "C001", "2025-2026-1", 101.0, 50.0, 50.0)); });
    assertThrowsWeek13<ValidationException>("B2 score final=-1",
        [&]{ scoreSvc.upsert(adminSession, Score("S001", "C001", "2025-2026-1", 50.0, -1.0, 50.0)); });
    assertThrowsWeek13<ValidationException>("B3 score empty studentId",
        [&]{ scoreSvc.upsert(adminSession, Score("", "C001", "2025-2026-1", 50.0, 50.0, 50.0)); });
    assertThrowsWeek13<ValidationException>("B4 student empty id",
        [&]{ studentSvc.create(adminSession, Student("", "X", "0", "Y", "Z", 2025)); });
    assertThrowsWeek13<ValidationException>("B5 student year<=0",
        [&]{ studentSvc.create(adminSession, Student("S_TMP", "X", "0", "Y", "Z", 0)); });
    assertThrowsWeek13<ValidationException>("B6 course empty id",
        [&]{ courseSvc.create(adminSession, Course("", "X", 3.0, "T001", "2025-2026-1")); });
    assertThrowsWeek13<ValidationException>("B7 course credit<=0",
        [&]{ courseSvc.create(adminSession, Course("C_TMP", "X", -3.0, "T001", "2025-2026-1")); });
    assertThrowsWeek13<ValidationException>("B8 course unknown teacher",
        [&]{ courseSvc.create(adminSession, Course("C_TMP", "X", 3.0, "T999", "2025-2026-1")); });

    std::cout << "  Group C (uniqueness):\n";
    assertThrowsWeek13<ValidationException>("C1 duplicate student S001",
        [&]{ studentSvc.create(adminSession, Student("S001", "dup", "0", "X", "Y", 2025)); });
    assertThrowsWeek13<ValidationException>("C2 duplicate course C001",
        [&]{ courseSvc.create(adminSession, Course("C001", "dup", 4.0, "T001", "2025-2026-1")); });

    std::cout << "  Group D (cascade residue):\n";
    {
        auto students = studentRepo.loadAll();
        if (std::any_of(students.begin(), students.end(),
                [](const Student& s) { return s.getId() == "S002"; })) {
            throw EduException("D1: S002 should be absent after Week 11 cascade");
        }
        std::cout << "   [D1 no ghost student S002] PASS\n";

        auto scores = scoreRepo.loadAll();
        if (std::any_of(scores.begin(), scores.end(),
                [](const Score& s) { return s.getStudentId() == "S002"; })) {
            throw EduException("D2: scores for S002 should be cascade-deleted");
        }
        std::cout << "   [D2 no ghost score for S002] PASS\n";

        auto users = userRepo.loadAll();
        if (std::any_of(users.begin(), users.end(),
                [](const UserAccount& u) {
                    return u.getRole() == RoleType::Student && u.getOwnerId() == "S002";
                })) {
            throw EduException("D3: s002 user account should be cascade-deleted");
        }
        std::cout << "   [D3 no ghost account for S002] PASS\n";
    }

    std::cout << "  Group E (teacher whitelist):\n";
    assertThrowsWeek13<PermissionException>("E1 ghost-teacher upsert C001",
        [&]{ scoreSvc.upsert(teacherGhost,
                Score("S001", "C001", "2025-2026-1", 50.0, 50.0, 50.0)); });
    assertThrowsWeek13<PermissionException>("E2 ghost-teacher findByCourse C001",
        [&]{ (void)scoreSvc.findByCourse(teacherGhost, "C001"); });
    assertThrowsWeek13<PermissionException>("E2b ghost-teacher findCourse C001",
        [&]{ (void)courseSvc.findById(teacherGhost, "C001"); });
    {
        auto ownCourses = courseSvc.listAll(teacherT001);
        if (!std::all_of(ownCourses.begin(), ownCourses.end(),
                [](const Course& c) { return c.getTeacherId() == "T001"; })) {
            throw EduException("E3: teacher listAll leaked non-own course");
        }
        std::cout << "   [E3 teacher course whitelist] PASS (n="
                  << ownCourses.size() << ")\n";

        auto ownScores = scoreSvc.listAll(teacherT001);
        auto courses   = courseRepo.loadAll();
        for (const auto& s : ownScores) {
            auto it = std::find_if(courses.begin(), courses.end(),
                [&](const Course& c) { return c.getCourseId() == s.getCourseId(); });
            if (it == courses.end() || it->getTeacherId() != "T001") {
                throw EduException("E4: teacher score listAll leaked non-own course");
            }
        }
        std::cout << "   [E4 teacher score whitelist] PASS (n="
                  << ownScores.size() << ")\n";
    }

    assertThrowsWeek13<PermissionException>("E5 student cannot read others",
        [&]{ (void)scoreSvc.findByStudent(studentSession, "S004"); });

    Logger::instance().info("Week 13 boundary check PASSED.");
    std::cout << " Week 13         : boundary-check PASSED\n";
}

int runInteractiveLoop(EduSys::AppContext& appContext) {
    using namespace EduSys;

    auto& authSvc        = appContext.authService;
    auto& studentSvc     = appContext.studentService;
    auto& courseSvc      = appContext.courseService;
    auto& scoreSvc       = appContext.scoreService;
    auto& statsSvc       = appContext.statsService;
    auto& reportExporter = appContext.reportExporter;

    constexpr int kMaxFailures = 3;
    int consecutiveFailures = 0;

    std::cout << "\n"
              << "==============================================\n"
              << " EduSys  -  Login\n"
              << " Tip: leave username empty to quit program.\n"
              << "==============================================\n";

    while (consecutiveFailures < kMaxFailures) {
        std::cout << "\n-- Login --\n";
        std::cout << "Username (empty = quit): ";
        std::cout.flush();

        std::string username;
        if (!std::getline(std::cin, username)) {
            std::cout << "\n(EOF, bye)\n";
            return 0;
        }
        while (!username.empty() && (username.back() == '\r' || username.back() == '\n')) {
            username.pop_back();
        }
        if (username.empty()) {
            std::cout << "Bye.\n";
            return 0;
        }

        std::cout << "Password: ";
        std::cout.flush();

        std::string password;
        std::getline(std::cin, password);
        while (!password.empty() && (password.back() == '\r' || password.back() == '\n')) {
            password.pop_back();
        }

        try {
            UserAccount acc = authSvc.authenticate(username, password);
            Session session;
            session.login(acc.getUsername(), acc.getRole(), acc.getOwnerId());
            consecutiveFailures = 0;

            std::cout << "[OK]  Welcome, " << acc.getUsername()
                      << " (role=" << (acc.getRole() == RoleType::Admin   ? "Admin"
                                     : acc.getRole() == RoleType::Teacher ? "Teacher"
                                                                          : "Student")
                      << ")\n";

            switch (acc.getRole()) {
                case RoleType::Admin: {
                    AdminMenu menu(session, authSvc, studentSvc, courseSvc, scoreSvc,
                                   statsSvc, reportExporter);
                    menu.run();
                    break;
                }
                case RoleType::Teacher: {
                    TeacherMenu menu(session, authSvc, courseSvc, scoreSvc, statsSvc);
                    menu.run();
                    break;
                }
                case RoleType::Student: {
                    StudentMenu menu(session, authSvc, studentSvc, scoreSvc, statsSvc);
                    menu.run();
                    break;
                }
            }
            std::cout << "\n[OK]  Logged out.\n";
        } catch (const AuthException& e) {
            ++consecutiveFailures;
            std::cout << "[ERR] " << e.what()
                      << "  (attempt " << consecutiveFailures << "/" << kMaxFailures << ")\n";
        } catch (const EduException& e) {
            std::cout << "[ERR] " << e.what() << "\n";
        }
    }

    std::cout << "Too many failed attempts. Exiting.\n";
    Logger::instance().warn("Login aborted after "
                            + std::to_string(kMaxFailures) + " consecutive failures.");
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    bool selfTest = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--self-test") {
            selfTest = true;
        } else {
            std::cerr << "Unknown argument: " << arg
                      << "  (supported: --self-test)\n";
            return 2;
        }
    }

    try {
        auto& logger = EduSys::Logger::instance();
        logger.info(std::string("EduSys starting up (mode=")
                    + (selfTest ? "self-test" : "interactive") + ").");

        EduSys::AppContext appContext;
        const bool seededThisRun = appContext.initializeData();

        if (selfTest) {
            std::cout << "==============================================\n"
                      << " EduSys - Week 11 + Week 13 Self-Test (--self-test)\n"
                      << " Run mode        : " << (seededThisRun ? "SEEDED (first run)" : "LOADED (persisted)") << "\n";
            runWeek11SelfCheck(appContext.studentRepo,
                               appContext.teacherRepo,
                               appContext.userRepo,
                               appContext.courseRepo,
                               appContext.scoreRepo,
                               seededThisRun);
            std::cout << " Week 11         : self-check PASSED\n";
            runWeek13BoundaryCheck(appContext.studentRepo,
                                   appContext.teacherRepo,
                                   appContext.userRepo,
                                   appContext.courseRepo,
                                   appContext.scoreRepo);
            std::cout << "==============================================\n";
            logger.info("EduSys shutdown normally (self-test).");
            return 0;
        }

        std::cout << "==============================================\n"
                  << " EduSys - Student Score Management System\n"
                  << " Week 12 build: interactive menus + stats\n"
                  << " Run mode        : " << (seededThisRun ? "SEEDED (first run)" : "LOADED (persisted)") << "\n"
                  << "==============================================\n";

        const int rc = runInteractiveLoop(appContext);
        logger.info("EduSys shutdown normally (interactive).");
        return rc;
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << '\n';
        try {
            EduSys::Logger::instance().error(std::string("Fatal: ") + e.what());
        } catch (...) {
            // Swallow secondary logging failures during fatal shutdown.
        }
        return 1;
    }
}
