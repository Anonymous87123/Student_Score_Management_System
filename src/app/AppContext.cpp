#include "EduSys/app/AppContext.hpp"

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
        Logger::instance().info("Existing data files detected -> skip seeding.");
        return false;
    }

    Logger::instance().info("All repositories empty -> seeding initial sample data.");
    seedSampleData(studentRepo, teacherRepo, userRepo, courseRepo, scoreRepo);
    return true;
}

} // namespace EduSys
