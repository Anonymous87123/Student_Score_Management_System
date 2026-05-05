#pragma once

#include <fstream>
#include <mutex>
#include <string>

namespace EduSys {

enum class LogLevel {
    Info,
    Warn,
    Error
};

// Synchronous append-only file logger for the single-process CLI app.
class Logger {
public:
    static Logger& instance();

    void log(LogLevel level, const std::string& msg);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();

    std::ofstream out_;
    std::mutex mu_;
};

} // namespace EduSys
