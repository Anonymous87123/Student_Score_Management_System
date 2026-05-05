#include "EduSys/common/Logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

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

} // namespace

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::Logger() {
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
