#include "EduSys/common/Logger.hpp"

#include <chrono>
#include <ctime>
#include <cerrno>
#include <iomanip>
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

namespace EduSys {

namespace {

std::string currentTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto tt  = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &tt);
#else
    localtime_r(&tt, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

const char* levelTag(LogLevel level) {
    switch (level) {
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "INFO";
}

std::string formatErrno(int err) {
    return "errno=" + std::to_string(err);
}

void ensureDirectoryExists(const std::string& path) {
    if (path.empty()) {
        return;
    }

    if (EDUSYS_MKDIR(path.c_str()) == 0) {
        return;
    }

    const int mkdirErr = errno;
    if (mkdirErr != EEXIST) {
        throw StorageException("Failed to create log directory '" + path + "' (" +
                               formatErrno(mkdirErr) + ")");
    }

    EDUSYS_STAT_STRUCT dirInfo{};
    if (EDUSYS_STAT(path.c_str(), &dirInfo) != 0) {
        throw StorageException("Path '" + path + "' exists but could not be inspected (" +
                               formatErrno(errno) + ")");
    }
    if (!EDUSYS_ISDIR(dirInfo.st_mode)) {
        throw StorageException("Path '" + path + "' exists but is not a directory.");
    }
}

void ensureLogParentDirectory() {
    const std::string logPath(LOG_FILE);
    const auto pos = logPath.find_last_of("/\\");
    if (pos == std::string::npos) {
        return;
    }
    ensureDirectoryExists(logPath.substr(0, pos));
}

} // namespace

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::Logger() {
    ensureLogParentDirectory();

    out_.open(LOG_FILE, std::ios::out | std::ios::app);
    if (!out_.is_open()) {
        throw StorageException(std::string("Failed to open log file: ") + LOG_FILE);
    }
}

Logger::~Logger() {
    if (out_.is_open()) {
        out_.flush();
        out_.close();
    }
}

void Logger::log(LogLevel level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mu_);
    if (!out_.is_open()) {
        return;
    }
    out_ << '[' << currentTimestamp() << "] "
         << '[' << levelTag(level) << "] "
         << msg << '\n';
    out_.flush();
}

void Logger::info(const std::string& msg)  { log(LogLevel::Info,  msg); }
void Logger::warn(const std::string& msg)  { log(LogLevel::Warn,  msg); }
void Logger::error(const std::string& msg) { log(LogLevel::Error, msg); }

} // namespace EduSys
