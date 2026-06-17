#include "EduSys/app/AppContext.hpp"

#include <algorithm>
#include <cerrno>
#include <string>
#include <vector>

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

namespace {

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

constexpr const char* kSeedAdminPlain   = "admin123";
constexpr const char* kSeedTeacherPlain = "t001pw";
constexpr const char* kSeedStudentPlain = "s001pw";

template <typename T, typename Pred>
bool containsMatching(const std::vector<T>& values, Pred pred) {
    return std::any_of(values.begin(), values.end(), pred);
}

const char* demoStudentNameForNumber(int n) {
    static const std::vector<const char*> names = {
        u8"白起", u8"嫦娥", u8"程咬金", u8"达摩", u8"东皇太一",
        u8"盾山", u8"铠", u8"廉颇", u8"刘邦", u8"刘禅",
        u8"吕布", u8"蒙恬", u8"芈月", u8"牛魔", u8"苏烈",
        u8"孙策", u8"太乙真人", u8"夏侯惇", u8"项羽", u8"亚瑟",
        u8"张飞", u8"钟无艳", u8"猪八戒", u8"庄周", u8"阿古朵",
        u8"曹操", u8"大司命", u8"典韦", u8"关羽", u8"花木兰",
        u8"姬小满", u8"橘右京", u8"狂铁", u8"老夫子", u8"李信",
        u8"刘备", u8"露娜", u8"马超", u8"梦奇", u8"墨子",
        u8"哪吒", u8"盘古", u8"裴擒虎", u8"司空震", u8"孙悟空",
        u8"夏洛特", u8"亚连", u8"雅典娜", u8"杨戬", u8"曜",
        u8"云缨", u8"云中君", u8"赵云", u8"阿轲", u8"百里玄策",
        u8"不知火舞", u8"貂蝉", u8"韩信", u8"镜", u8"兰陵王",
        u8"澜", u8"李白", u8"娜可露露", u8"上官婉儿", u8"司马懿",
        u8"元歌", u8"安琪拉", u8"扁鹊", u8"妲己", u8"干将莫邪",
        u8"高渐离", u8"海诺", u8"海月", u8"姜子牙", u8"金蝉",
        u8"米莱狄", u8"女娲", u8"沈梦溪", u8"孙膑", u8"王昭君",
        u8"武则天", u8"西施", u8"小乔", u8"杨玉环", u8"弈星",
        u8"嬴政", u8"张良", u8"甄姬", u8"钟馗", u8"周瑜",
        u8"诸葛亮", u8"艾琳", u8"敖隐", u8"百里守约", u8"苍",
        u8"狄仁杰", u8"伽罗", u8"戈娅", u8"公孙离", u8"后羿"
    };

    const int index = (n == 1) ? 0 : (n - 2);
    if (index < 0 || static_cast<std::size_t>(index) >= names.size()) {
        return u8"学生";
    }
    return names[static_cast<std::size_t>(index)];
}

bool ensureDemoCatalog(EduSys::StudentRepository& sr,
                       EduSys::TeacherRepository& tr,
                       EduSys::UserRepository& ur,
                       EduSys::CourseRepository& cr,
                       EduSys::ScoreRepository& scr) {
    using namespace EduSys;

    bool changed = false;

    auto students = sr.loadAll();
    auto teachers = tr.loadAll();
    auto users = ur.loadAll();
    auto courses = cr.loadAll();
    auto scores = scr.loadAll();

    auto addStudent = [&](const char* id,
                          const char* name,
                          const char* contact,
                          const char* major,
                          const char* className,
                          int enrollYear) {
        if (containsMatching(students, [&](const Student& s) { return s.getId() == id; })) {
            return;
        }
        students.emplace_back(id, name, contact, major, className, enrollYear);
        changed = true;
    };

    auto addTeacher = [&](const char* id,
                          const char* name,
                          const char* contact,
                          const char* department,
                          const char* title) {
        if (containsMatching(teachers, [&](const Teacher& t) { return t.getId() == id; })) {
            return;
        }
        teachers.emplace_back(id, name, contact, department, title);
        changed = true;
    };

    auto addUser = [&](const char* username,
                       const char* plainPassword,
                       RoleType role,
                       const char* ownerId) {
        if (containsMatching(users, [&](const UserAccount& u) { return u.getUsername() == username; })) {
            return;
        }
        users.emplace_back(username, PasswordHasher::hash(plainPassword), role, ownerId, true);
        changed = true;
    };

    auto addCourse = [&](const char* id,
                         const char* name,
                         double credit,
                         const char* teacherId,
                         const char* semester) {
        if (containsMatching(courses, [&](const Course& c) { return c.getCourseId() == id; })) {
            return;
        }
        courses.emplace_back(id, name, credit, teacherId, semester);
        changed = true;
    };

    auto addScore = [&](const char* studentId,
                        const char* courseId,
                        const char* semester,
                        double usual,
                        double finalScore,
                        double total) {
        if (containsMatching(scores, [&](const Score& s) {
                return s.getStudentId() == studentId &&
                       s.getCourseId() == courseId &&
                       s.getSemester() == semester;
            })) {
            return;
        }
        scores.emplace_back(studentId, courseId, semester, usual, finalScore, total);
        changed = true;
    };

    auto paddedNumber = [](int n) {
        if (n < 10) {
            return "00" + std::to_string(n);
        }
        if (n < 100) {
            return "0" + std::to_string(n);
        }
        return std::to_string(n);
    };

    const std::vector<std::string> legacyShortStudentIds = {
        "S03", "S04", "S05", "S06", "S07", "S08", "S09"
    };
    const auto isLegacyShortId = [&](const std::string& id) {
        return containsMatching(legacyShortStudentIds, [&](const std::string& legacy) {
            return legacy == id;
        });
    };
    const auto studentsBeforeLegacyCleanup = students.size();
    students.erase(std::remove_if(students.begin(), students.end(),
        [&](const Student& s) {
            return s.getId() == "S002" || isLegacyShortId(s.getId());
        }),
        students.end());
    const auto usersBeforeLegacyCleanup = users.size();
    users.erase(std::remove_if(users.begin(), users.end(),
        [&](const UserAccount& u) {
            return u.getRole() == RoleType::Student &&
                   (u.getOwnerId() == "S002" || isLegacyShortId(u.getOwnerId()));
        }),
        users.end());
    const auto scoresBeforeLegacyCleanup = scores.size();
    scores.erase(std::remove_if(scores.begin(), scores.end(),
        [&](const Score& s) {
            return s.getStudentId() == "S002" || isLegacyShortId(s.getStudentId());
        }),
        scores.end());
    if (students.size() != studentsBeforeLegacyCleanup ||
        users.size() != usersBeforeLegacyCleanup ||
        scores.size() != scoresBeforeLegacyCleanup) {
        changed = true;
    }

    addStudent("S001", demoStudentNameForNumber(1), "13800000001", "Computer Science", "CS2501", 2025);
    addStudent("S003", demoStudentNameForNumber(3), "13800000003", "Computer Science", "CS2501", 2025);
    addStudent("S004", demoStudentNameForNumber(4), "13800000004", "Computer Science", "CS2501", 2025);
    addStudent("S005", demoStudentNameForNumber(5), "13800000005", "Software Engineering", "SE2501", 2025);
    addStudent("S006", demoStudentNameForNumber(6), "13800000006", "Software Engineering", "SE2501", 2025);
    addStudent("S007", demoStudentNameForNumber(7), "13800000007", "Artificial Intelligence", "AI2501", 2025);
    addStudent("S008", demoStudentNameForNumber(8), "13800000008", "Artificial Intelligence", "AI2501", 2025);
    addStudent("S009", demoStudentNameForNumber(9), "13800000009", "Data Science", "DS2501", 2025);
    addStudent("S010", demoStudentNameForNumber(10), "13800000010", "Data Science", "DS2501", 2025);
    addStudent("S011", demoStudentNameForNumber(11), "13800000011", "Computer Science", "CS2502", 2025);
    addStudent("S012", demoStudentNameForNumber(12), "13800000012", "Computer Science", "CS2502", 2025);
    addStudent("S013", demoStudentNameForNumber(13), "13800000013", "Software Engineering", "SE2502", 2025);
    addStudent("S014", demoStudentNameForNumber(14), "13800000014", "Software Engineering", "SE2502", 2025);
    addStudent("S015", demoStudentNameForNumber(15), "13800000015", "Artificial Intelligence", "AI2502", 2025);
    addStudent("S016", demoStudentNameForNumber(16), "13800000016", "Artificial Intelligence", "AI2502", 2025);
    addStudent("S017", demoStudentNameForNumber(17), "13800000017", "Data Science", "DS2502", 2025);
    addStudent("S018", demoStudentNameForNumber(18), "13800000018", "Data Science", "DS2502", 2025);
    addStudent("S019", demoStudentNameForNumber(19), "13800000019", "Computer Science", "CS2503", 2025);
    addStudent("S020", demoStudentNameForNumber(20), "13800000020", "Computer Science", "CS2503", 2025);
    addStudent("S021", demoStudentNameForNumber(21), "13800000021", "Software Engineering", "SE2503", 2025);
    addStudent("S022", demoStudentNameForNumber(22), "13800000022", "Software Engineering", "SE2503", 2025);
    addStudent("S023", demoStudentNameForNumber(23), "13800000023", "Artificial Intelligence", "AI2503", 2025);
    addStudent("S024", demoStudentNameForNumber(24), "13800000024", "Artificial Intelligence", "AI2503", 2025);
    addStudent("S025", demoStudentNameForNumber(25), "13800000025", "Data Science", "DS2503", 2025);
    addStudent("S026", demoStudentNameForNumber(26), "13800000026", "Data Science", "DS2503", 2025);
    addStudent("S027", demoStudentNameForNumber(27), "13800000027", "Computer Science", "CS2504", 2025);
    addStudent("S028", demoStudentNameForNumber(28), "13800000028", "Computer Science", "CS2504", 2025);
    addStudent("S029", demoStudentNameForNumber(29), "13800000029", "Software Engineering", "SE2504", 2025);
    addStudent("S030", demoStudentNameForNumber(30), "13800000030", "Software Engineering", "SE2504", 2025);

    const std::vector<std::string> generatedMajors = {
        "Computer Science",
        "Software Engineering",
        "Artificial Intelligence",
        "Data Science",
        "Internet of Things Engineering",
        "Cyber Security",
        "Electronic Information Engineering"
    };
    const std::vector<std::string> generatedClasses = {
        "CS2505", "CS2506", "SE2505", "SE2506", "AI2504", "AI2505",
        "DS2504", "DS2505", "IOT2501", "IOT2502", "CY2501", "EE2501"
    };
    for (int n = 31; n <= 101; ++n) {
        const std::string number = paddedNumber(n);
        const std::string id = "S" + number;
        const char* name = demoStudentNameForNumber(n);
        const std::string contact = "13800000" + number;
        const std::string& major = generatedMajors[static_cast<std::size_t>(n) % generatedMajors.size()];
        const std::string& className = generatedClasses[static_cast<std::size_t>(n) % generatedClasses.size()];
        addStudent(id.c_str(), name, contact.c_str(), major.c_str(), className.c_str(), 2025);
    }

    addTeacher("T001", "Xu Hongyun", "t001@scut.edu.cn", "School of CSE", "Professor");
    addTeacher("T002", "Chen Wei", "t002@scut.edu.cn", "School of Mathematics", "Professor");
    addTeacher("T003", "Liu Fang", "t003@scut.edu.cn", "School of Mathematics", "Associate Professor");
    addTeacher("T004", "Wang Jian", "t004@scut.edu.cn", "School of Physics", "Professor");
    addTeacher("T005", "Zhao Qiang", "t005@scut.edu.cn", "School of CSE", "Associate Professor");
    addTeacher("T006", "Lin Mei", "t006@scut.edu.cn", "School of Engineering", "Lecturer");

    addUser("admin", kSeedAdminPlain, RoleType::Admin, "");
    addUser("t001", "t001pw", RoleType::Teacher, "T001");
    addUser("t002", "t002pw", RoleType::Teacher, "T002");
    addUser("t003", "t003pw", RoleType::Teacher, "T003");
    addUser("t004", "t004pw", RoleType::Teacher, "T004");
    addUser("t005", "t005pw", RoleType::Teacher, "T005");
    addUser("t006", "t006pw", RoleType::Teacher, "T006");

    std::vector<std::string> studentIds;
    studentIds.emplace_back("S001");
    for (int n = 3; n <= 101; ++n) {
        const std::string number = paddedNumber(n);
        studentIds.emplace_back("S" + number);
    }
    for (const auto& sid : studentIds) {
        const std::string username = "s" + sid.substr(1);
        const std::string password = username + "pw";
        addUser(username.c_str(), password.c_str(), RoleType::Student, sid.c_str());
    }

    addCourse("C001", "Advanced Programming (C++)", 4.0, "T001", "2025-2026-1");
    addCourse("C002", u8"工科数学分析 I", 5.0, "T002", "2025-2026-1");
    addCourse("C003", u8"工科数学分析 II", 5.0, "T002", "2025-2026-2");
    addCourse("C004", u8"线性代数", 3.0, "T002", "2025-2026-1");
    addCourse("C005", u8"概率论与数理统计", 3.5, "T003", "2025-2026-2");
    addCourse("C006", u8"大学物理 I", 4.0, "T004", "2025-2026-1");
    addCourse("C007", u8"大学物理 II", 4.0, "T004", "2025-2026-2");
    addCourse("C008", u8"大学物理实验", 1.5, "T004", "2025-2026-2");
    addCourse("C009", u8"C++ 程序设计", 4.0, "T001", "2025-2026-1");
    addCourse("C010", u8"数据结构", 4.0, "T001", "2025-2026-2");
    addCourse("C011", u8"离散数学", 3.0, "T003", "2025-2026-1");
    addCourse("C012", u8"计算机组成原理", 4.0, "T005", "2025-2026-2");
    addCourse("C013", u8"操作系统", 4.0, "T005", "2026-2027-1");
    addCourse("C014", u8"数据库系统", 3.5, "T006", "2026-2027-1");
    addCourse("C015", u8"电路与电子技术", 3.5, "T004", "2025-2026-1");
    addCourse("C016", u8"工程制图", 2.5, "T006", "2025-2026-1");
    addCourse("C017", u8"大学英语", 2.0, "T006", "2025-2026-1");
    addCourse("C018", u8"Python 数据分析", 3.0, "T001", "2026-2027-1");
    addCourse("C019", u8"面向对象程序设计", 3.5, "T001", "2025-2026-2");
    addCourse("C020", u8"软件工程导论", 3.0, "T005", "2026-2027-1");

    const std::vector<std::string> scoreCourseIds = {
        "C001", "C002", "C004", "C005", "C006", "C009", "C010", "C011"
    };
    for (std::size_t si = 0; si < studentIds.size(); ++si) {
        const auto& sid = studentIds[si];
        const std::size_t courseLimit = si < 20 ? scoreCourseIds.size() : 5;
        for (std::size_t ci = 0; ci < courseLimit; ++ci) {
            double usual = 72.0 + static_cast<double>((si * 7 + ci * 3) % 24);
            double finalScore = 70.0 + static_cast<double>((si * 5 + ci * 4) % 26);
            double total = usual * 0.4 + finalScore * 0.6;

            if (sid == "S004" && scoreCourseIds[ci] == "C002") {
                usual = 48.0;
                finalScore = 52.0;
                total = 50.4;
            }

            addScore(sid.c_str(), scoreCourseIds[ci].c_str(), "2025-2026-1",
                     usual, finalScore, total);
        }
    }

    if (changed) {
        sr.saveAll(students);
        tr.saveAll(teachers);
        ur.saveAll(users);
        cr.saveAll(courses);
        scr.saveAll(scores);
    }
    return changed;
}

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

    students.emplace_back("S001", demoStudentNameForNumber(1), "13800000001", "Computer Science", "CS2501", 2025);

    teachers.emplace_back("T001", "Xu Hongyun", "t001@scut", "School of CSE", "Professor");

    const std::string adminHash   = PasswordHasher::hash(kSeedAdminPlain);
    const std::string teacherHash = PasswordHasher::hash(kSeedTeacherPlain);
    const std::string studentHash = PasswordHasher::hash(kSeedStudentPlain);

    users.emplace_back("admin", adminHash,   RoleType::Admin,   "",     true);
    users.emplace_back("t001",  teacherHash, RoleType::Teacher, "T001", true);
    users.emplace_back("s001",  studentHash, RoleType::Student, "S001", true);

    courses.emplace_back("C001", "Advanced Programming (C++)", 4.0, "T001", "2025-2026-1");

    scores.emplace_back("S001", "C001", "2025-2026-1", 85.0, 90.0, 88.5);

    sr.saveAll(students);
    tr.saveAll(teachers);
    ur.saveAll(users);
    cr.saveAll(courses);
    scr.saveAll(scores);

    ensureDemoCatalog(sr, tr, ur, cr, scr);
}

} // namespace

namespace EduSys {

AppContext::AppContext()
    : studentRepo()
    , teacherRepo()
    , userRepo()
    , courseRepo()
    , scoreRepo()
    , authService(userRepo)
    , studentService(studentRepo, scoreRepo, userRepo)
    , teacherService(teacherRepo, courseRepo, scoreRepo, userRepo)
    , courseService(courseRepo, scoreRepo, teacherRepo)
    , scoreService(scoreRepo, courseRepo, studentRepo)
    , statsService(studentRepo, courseRepo, scoreRepo)
    , reportExporter(statsService) {}

bool AppContext::initializeData() {
    ensureDirectoryExists(DATA_DIR);

    const bool allEmpty =
        studentRepo.loadAll().empty() && teacherRepo.loadAll().empty() &&
        userRepo.loadAll().empty()    && courseRepo.loadAll().empty()  &&
        scoreRepo.loadAll().empty();

    if (!allEmpty) {
        Logger::instance().info("Existing data files detected -> loading persisted data without demo refill.");
        return false;
    }

    Logger::instance().info("All repositories empty -> seeding initial sample data.");
    seedSampleData(studentRepo, teacherRepo, userRepo, courseRepo, scoreRepo);
    return true;
}

} // namespace EduSys
