#include <algorithm>
#include <cerrno>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#  include <direct.h>
#  include <sys/stat.h>
#  define EDUSYS_MKDIR(p) _mkdir(p)
#  define EDUSYS_STAT(path, info) _stat((path), (info))
#  define EDUSYS_STAT_STRUCT struct _stat
#  define EDUSYS_ISDIR(mode) (((mode) & _S_IFDIR) != 0)
#else
#  include <sys/stat.h>
#  include <sys/types.h>
#  define EDUSYS_MKDIR(p) mkdir((p), 0755)
#  define EDUSYS_STAT(path, info) stat((path), (info))
#  define EDUSYS_STAT_STRUCT struct stat
#  define EDUSYS_ISDIR(mode) S_ISDIR(mode)
#endif

#include "EduSys/common/Constants.hpp"
#include "EduSys/common/Exception.hpp"
#include "EduSys/common/Logger.hpp"
#include "EduSys/common/PasswordHasher.hpp"
#include "EduSys/common/Types.hpp"
#include "EduSys/model/Course.hpp"
#include "EduSys/model/Score.hpp"
#include "EduSys/model/Student.hpp"
#include "EduSys/model/Teacher.hpp"
#include "EduSys/model/UserAccount.hpp"
#include "EduSys/report/ReportExporter.hpp"
#include "EduSys/service/AuthService.hpp"
#include "EduSys/service/CourseService.hpp"
#include "EduSys/service/ScoreService.hpp"
#include "EduSys/service/Session.hpp"
#include "EduSys/service/StatsService.hpp"
#include "EduSys/service/StudentService.hpp"
#include "EduSys/storage/CourseRepository.hpp"
#include "EduSys/storage/ScoreRepository.hpp"
#include "EduSys/storage/StudentRepository.hpp"
#include "EduSys/storage/TeacherRepository.hpp"
#include "EduSys/storage/UserRepository.hpp"
#include "EduSys/view/AdminMenu.hpp"
#include "EduSys/view/StudentMenu.hpp"
#include "EduSys/view/TeacherMenu.hpp"

namespace {

// 避开 MSVC C4996（std::strerror 视作 deprecated）；课程项目中暴露 errno 数值已足够定位。
std::string formatErrno(int err) {
    return "errno=" + std::to_string(err);
}

void ensureDirectoryExists(const char* path) {
    if (EDUSYS_MKDIR(path) == 0) {
        return;
    }

    const int mkdirErr = errno;
    if (mkdirErr != EEXIST) {
        throw EduSys::StorageException(
            std::string("Failed to create data directory '") + path + "' (" + formatErrno(mkdirErr) + ")");
    }

    EDUSYS_STAT_STRUCT dirInfo{};
    if (EDUSYS_STAT(path, &dirInfo) != 0) {
        throw EduSys::StorageException(
            std::string("Path '") + path + "' exists but could not be inspected (" + formatErrno(errno) + ")");
    }

    if (!EDUSYS_ISDIR(dirInfo.st_mode)) {
        throw EduSys::StorageException(
            std::string("Path '") + path + "' exists but is not a directory.");
    }
}

// Week 11：真实哈希替换 Week 10 占位。固定盐 + FNV-1a -> 16 字符 hex，详见 PasswordHasher。
// 明文仅用于初始化样例；真实运行中用户可通过 AuthService::changePassword 修改自己的密码。
const std::string kSeedAdminPlain   = "admin123";
const std::string kSeedTeacherPlain = "t001pw";
const std::string kSeedStudentPlain = "s001pw";

void seedSampleData(EduSys::StudentRepository& sr,
                    EduSys::TeacherRepository& tr,
                    EduSys::UserRepository& ur,
                    EduSys::CourseRepository& cr,
                    EduSys::ScoreRepository& scr) {
    using namespace EduSys;

    std::vector<Student>     students;
    std::vector<Teacher>     teachers;
    std::vector<UserAccount> users;
    std::vector<Course>      courses;
    std::vector<Score>       scores;

    students.emplace_back("S001", "Zhang San", "13800000001", "Computer Science", "CS2501", 2025);
    students.emplace_back("S002", "Li Si",     "13800000002", "Computer Science", "CS2501", 2025);

    teachers.emplace_back("T001", "Xu Hongyun", "t001@scut", "School of CSE", "Professor");

    const std::string adminHash   = PasswordHasher::hash(kSeedAdminPlain);
    const std::string teacherHash = PasswordHasher::hash(kSeedTeacherPlain);
    const std::string studentHash = PasswordHasher::hash(kSeedStudentPlain);

    users.emplace_back("admin", adminHash,   RoleType::Admin,   "",     true);
    users.emplace_back("t001",  teacherHash, RoleType::Teacher, "T001", true);
    users.emplace_back("s001",  studentHash, RoleType::Student, "S001", true);
    users.emplace_back("s002",  studentHash, RoleType::Student, "S002", true);

    courses.emplace_back("C001", "Advanced Programming (C++)", 4.0, "T001", "2025-2026-1");

    scores.emplace_back("S001", "C001", "2025-2026-1", 85.0, 90.0, 88.5);
    scores.emplace_back("S002", "C001", "2025-2026-1", 70.0, 75.0, 73.5);

    sr.saveAll(students);
    tr.saveAll(teachers);
    ur.saveAll(users);
    cr.saveAll(courses);
    scr.saveAll(scores);
}

// Week 11 自检：把 AuthService / Session / 三个领域 Service 的核心路径全部跑通一次。
// 设计取舍（用户已确认 A 选项）：
//   - 自检会真实改写 .dat（删 S002、加 S003、新增成绩），不复原；
//   - 但破坏性写操作只在"刚刚 seed 完"那一次执行，避免第二次启动时撞上 ValidationException；
//   - 只读 / 鉴权 / 权限拒绝 / 字段越界这些路径每次启动都跑，确保回归性始终被验证。
//
// 任一断言不成立直接抛异常，由 main() 的 catch 兜底退出码 1。
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

    // ---------- 1. 认证：正向 ----------
    UserAccount adminAcc = authSvc.authenticate("admin", kSeedAdminPlain);
    std::cout << " [Week11] AUTH OK            : admin (role=Admin)\n";

    Session adminSession;
    adminSession.login(adminAcc.getUsername(), adminAcc.getRole(), adminAcc.getOwnerId());

    // ---------- 2. 认证：错误密码必须被拒 ----------
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

    // ---------- 3. 权限拒绝：以学生身份尝试 create 必须抛 PermissionException ----------
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

    // ---------- 4. 范围校验：成绩越界必须抛 ValidationException ----------
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

    // ---------- 5. 教师写不属于自己的课程必须被拒 ----------
    // (此项始终可跑：t001 是 C001 的授课老师，构造一个不存在的课程 id 触发拒绝路径)
    Session teacherSession;
    teacherSession.login("t001", RoleType::Teacher, "T001");

    // ---------- 6. 破坏性写操作：仅首次 seed 后跑一次 ----------
    if (seededThisRun) {
        // 6a. 创建新学生 S003
        Student s003("S003", "Wang Wu", "13800000003", "Computer Science", "CS2501", 2025);
        studentSvc.create(adminSession, s003);

        // 6b. 教师 t001 给 S003 录 C001 成绩（教师写自己授课的课程，应通过）
        Score s003Score("S003", "C001", "2025-2026-1", 92.0, 95.0, 94.1);
        scoreSvc.upsert(teacherSession, s003Score);

        // 6c. 管理员 upsert 已有 S001/C001 成绩（验证 update 路径）
        Score s001Updated("S001", "C001", "2025-2026-1", 88.0, 92.0, 90.8);
        scoreSvc.upsert(adminSession, s001Updated);

        // 6d. 级联删除 S002：scores 中 S002/C001 应消失，users 中 s002 账号也应消失
        studentSvc.remove(adminSession, "S002");

        logger.info("Week 11 mutation block executed (S003 created, S001 score updated, S002 cascade-deleted).");
        std::cout << " [Week11] MUTATIONS done     : +S003, S001 score updated, -S002 (cascade)\n";
    } else {
        std::cout << " [Week11] MUTATIONS skipped  : LOADED run, verifying persisted state instead\n";
    }

    // ---------- 7. 最终状态断言：S002 不应存在，S003 必须存在 ----------
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

    if (hasStudent("S002"))     throw EduException("Self-check: S002 should be deleted");
    if (hasUserOwner("S002"))   throw EduException("Self-check: s002 user account should be deleted");
    if (hasScoreFor("S002"))    throw EduException("Self-check: scores for S002 should be cascade-deleted");
    if (!hasStudent("S003"))    throw EduException("Self-check: S003 should exist");
    if (!hasScoreFor("S003"))   throw EduException("Self-check: score for S003 should exist");

    std::cout << " [Week11] FINAL ASSERT ok    : S002 absent, S003 present, scores consistent\n";
    logger.info("Week 11 self-check PASSED.");
}

// Week 13 辅助：断言某 lambda 必须抛出指定异常类型；否则直接抛 EduException 让 main 兜底。
// 与 Week 11 散点 try/catch 相比，集中成模板更易读，也让用例名在输出中对齐。
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

// Week 13 边界自检：A-E 五组（A 认证/会话、B 字段校验、C 唯一性、D 级联残留、E 教师白名单）。
// 全部只读 / 纯拒绝路径，不触碰任何 .dat 文件；F 组文件损坏用例外置到 tools/corrupt_check.bat。
// 任一断言不通过立刻抛异常，由 main() 统一兜底为退出码 1。
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

    // 复用 Week 11 已经验证过的会话；Week 13 不再做"登录"动作，只构造各色身份证。
    Session adminSession;     adminSession.login("admin", RoleType::Admin, "");
    Session studentSession;   studentSession.login("s001", RoleType::Student, "S001");
    Session teacherT001;      teacherT001.login("t001", RoleType::Teacher, "T001");
    // ghost 教师：role=Teacher、ownerId 是不存在的 T999；用来打教师白名单的越权场景。
    Session teacherGhost;     teacherGhost.login("ghost", RoleType::Teacher, "T999");
    Session emptySession;     // 未登录态，所有写接口都应拒绝

    std::cout << "\n -- Week 13 boundary checks (A-E groups, read-only) --\n";

    // ===== Group A: 认证 / 会话 =====
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

    // ===== Group B: 字段校验 =====
    // 全部走 Service 入口，Service 在 saveAll 之前就抛异常，因此对 .dat 完全无副作用。
    std::cout << "  Group B (field validation):\n";
    assertThrowsWeek13<ValidationException>("B1 score usual=101",
        [&]{ scoreSvc.upsert(adminSession, Score("S001","C001","2025-2026-1",101.0,50.0,50.0)); });
    assertThrowsWeek13<ValidationException>("B2 score final=-1",
        [&]{ scoreSvc.upsert(adminSession, Score("S001","C001","2025-2026-1",50.0,-1.0,50.0)); });
    assertThrowsWeek13<ValidationException>("B3 score empty studentId",
        [&]{ scoreSvc.upsert(adminSession, Score("","C001","2025-2026-1",50.0,50.0,50.0)); });
    assertThrowsWeek13<ValidationException>("B4 student empty id",
        [&]{ studentSvc.create(adminSession, Student("","X","0","Y","Z",2025)); });
    assertThrowsWeek13<ValidationException>("B5 student year<=0",
        [&]{ studentSvc.create(adminSession, Student("S_TMP","X","0","Y","Z",0)); });
    assertThrowsWeek13<ValidationException>("B6 course empty id",
        [&]{ courseSvc.create(adminSession, Course("","X",3.0,"T001","2025-2026-1")); });
    assertThrowsWeek13<ValidationException>("B7 course credit<=0",
        [&]{ courseSvc.create(adminSession, Course("C_TMP","X",-3.0,"T001","2025-2026-1")); });
    assertThrowsWeek13<ValidationException>("B8 course unknown teacher",
        [&]{ courseSvc.create(adminSession, Course("C_TMP","X",3.0,"T999","2025-2026-1")); });

    // ===== Group C: 唯一性 =====
    std::cout << "  Group C (uniqueness):\n";
    assertThrowsWeek13<ValidationException>("C1 duplicate student S001",
        [&]{ studentSvc.create(adminSession, Student("S001","dup","0","X","Y",2025)); });
    assertThrowsWeek13<ValidationException>("C2 duplicate course C001",
        [&]{ courseSvc.create(adminSession, Course("C001","dup",4.0,"T001","2025-2026-1")); });

    // ===== Group D: 级联删除残留检查（验证 Week 11 mutation block 的最终状态） =====
    std::cout << "  Group D (cascade residue):\n";
    {
        auto students = studentRepo.loadAll();
        if (std::any_of(students.begin(), students.end(),
                [](const Student& s){ return s.getId() == "S002"; })) {
            throw EduException("D1: S002 should be absent after Week 11 cascade");
        }
        std::cout << "   [D1 no ghost student S002] PASS\n";

        auto scores = scoreRepo.loadAll();
        if (std::any_of(scores.begin(), scores.end(),
                [](const Score& s){ return s.getStudentId() == "S002"; })) {
            throw EduException("D2: scores for S002 should be cascade-deleted");
        }
        std::cout << "   [D2 no ghost score for S002] PASS\n";

        auto users = userRepo.loadAll();
        if (std::any_of(users.begin(), users.end(),
                [](const UserAccount& u){
                    return u.getRole() == RoleType::Student && u.getOwnerId() == "S002";
                })) {
            throw EduException("D3: s002 user account should be cascade-deleted");
        }
        std::cout << "   [D3 no ghost account for S002] PASS\n";
    }

    // ===== Group E: 教师白名单双重防线 =====
    std::cout << "  Group E (teacher whitelist):\n";
    assertThrowsWeek13<PermissionException>("E1 ghost-teacher upsert C001",
        [&]{ scoreSvc.upsert(teacherGhost,
                Score("S001","C001","2025-2026-1",50.0,50.0,50.0)); });
    assertThrowsWeek13<PermissionException>("E2 ghost-teacher findByCourse C001",
        [&]{ (void)scoreSvc.findByCourse(teacherGhost, "C001"); });
    {
        auto ownScores = scoreSvc.listAll(teacherT001);
        auto courses   = courseRepo.loadAll();
        for (const auto& s : ownScores) {
            auto it = std::find_if(courses.begin(), courses.end(),
                [&](const Course& c){ return c.getCourseId() == s.getCourseId(); });
            if (it == courses.end() || it->getTeacherId() != "T001") {
                throw EduException("E3: teacher listAll leaked non-own course");
            }
        }
        std::cout << "   [E3 teacher listAll whitelist] PASS (n="
                  << ownScores.size() << ")\n";
    }

    // 学生只读自己的成绩：再补一刀，证明 student 角色无法跨学号读
    assertThrowsWeek13<PermissionException>("E4 student cannot read others",
        [&]{ (void)scoreSvc.findByStudent(studentSession, "S004"); });

    Logger::instance().info("Week 13 boundary check PASSED.");
    std::cout << " Week 13         : boundary-check PASSED\n";
}

// Week 12 交互主循环：登录 3 次失败即正常退出；空用户名立即退出。
// 登录成功后按角色分派到 AdminMenu / TeacherMenu / StudentMenu，菜单跑完登出回到登录界面。
// 所有 Service / Exporter 在这里统一 new 一次、所有 Session 复用同一组 Service 引用。
int runInteractiveLoop(EduSys::StudentRepository& studentRepo,
                       EduSys::TeacherRepository& teacherRepo,
                       EduSys::UserRepository&    userRepo,
                       EduSys::CourseRepository&  courseRepo,
                       EduSys::ScoreRepository&   scoreRepo) {
    using namespace EduSys;

    AuthService    authSvc(userRepo);
    StudentService studentSvc(studentRepo, scoreRepo, userRepo);
    CourseService  courseSvc(courseRepo, scoreRepo, teacherRepo);
    ScoreService   scoreSvc(scoreRepo, courseRepo, studentRepo);
    StatsService   statsSvc(studentRepo, courseRepo, scoreRepo);
    ReportExporter reportExporter(statsSvc);

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

// Week 12: 默认进入交互登录循环；传 --self-test 则走 Week 11 回归自检路径。
// 无论走哪条路径，启动时都先保证 data/ 目录存在、五个仓储可构造、首次缺数据即 seed。
int main(int argc, char* argv[]) {
    bool selfTest = false;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--self-test") {
            selfTest = true;
        } else {
            std::cerr << "Unknown argument: " << a
                      << "  (supported: --self-test)\n";
            return 2;
        }
    }

    try {
        ensureDirectoryExists(EduSys::DATA_DIR);

        auto& logger = EduSys::Logger::instance();
        logger.info(std::string("EduSys starting up (mode=")
                    + (selfTest ? "self-test" : "interactive") + ").");

        EduSys::StudentRepository studentRepo;
        EduSys::TeacherRepository teacherRepo;
        EduSys::UserRepository    userRepo;
        EduSys::CourseRepository  courseRepo;
        EduSys::ScoreRepository   scoreRepo;

        const bool allEmpty =
            studentRepo.loadAll().empty() && teacherRepo.loadAll().empty() &&
            userRepo.loadAll().empty()    && courseRepo.loadAll().empty()  &&
            scoreRepo.loadAll().empty();

        bool seededThisRun = false;
        if (allEmpty) {
            logger.info("All repositories empty -> seeding initial sample data.");
            seedSampleData(studentRepo, teacherRepo, userRepo, courseRepo, scoreRepo);
            seededThisRun = true;
        } else {
            logger.info("Existing data files detected -> skip seeding.");
        }

        if (selfTest) {
            std::cout << "==============================================\n"
                      << " EduSys - Week 11 + Week 13 Self-Test (--self-test)\n"
                      << " Run mode        : " << (seededThisRun ? "SEEDED (first run)" : "LOADED (persisted)") << "\n";
            runWeek11SelfCheck(studentRepo, teacherRepo, userRepo, courseRepo, scoreRepo, seededThisRun);
            std::cout << " Week 11         : self-check PASSED\n";
            runWeek13BoundaryCheck(studentRepo, teacherRepo, userRepo, courseRepo, scoreRepo);
            std::cout << "==============================================\n";
            logger.info("EduSys shutdown normally (self-test).");
            return 0;
        }

        std::cout << "==============================================\n"
                  << " EduSys - Student Score Management System\n"
                  << " Week 12 build: interactive menus + stats\n"
                  << " Run mode        : " << (seededThisRun ? "SEEDED (first run)" : "LOADED (persisted)") << "\n"
                  << "==============================================\n";

        const int rc = runInteractiveLoop(studentRepo, teacherRepo, userRepo, courseRepo, scoreRepo);
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
