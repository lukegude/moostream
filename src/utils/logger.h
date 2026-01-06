#pragma once

#include <string>
#include <fstream>
#include <mutex>

namespace ytui {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static void init(const std::string& log_file = "");
    static void shutdown();

    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);

    static void set_level(LogLevel level);

private:
    static void log(LogLevel level, const std::string& message);
    static std::string level_to_string(LogLevel level);

    static std::ofstream log_file_;
    static LogLevel current_level_;
    static std::mutex mutex_;
};

} // namespace ytui
