#include "utils/logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace ytui {

std::ofstream Logger::log_file_;
LogLevel Logger::current_level_ = LogLevel::Info;
std::mutex Logger::mutex_;

void Logger::init(const std::string& log_file) {
    if (!log_file.empty()) {
        log_file_.open(log_file, std::ios::app);
    }
}

void Logger::shutdown() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::Info, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::Warning, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::Error, message);
}

void Logger::set_level(LogLevel level) {
    current_level_ = level;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < current_level_) return;

    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count()
       << " [" << level_to_string(level) << "] " << message;

    std::string log_line = ss.str();

    if (log_file_.is_open()) {
        log_file_ << log_line << std::endl;
    }

    // Also output to stderr for errors
    if (level == LogLevel::Error) {
        std::cerr << log_line << std::endl;
    }
}

std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
        default: return "UNKNOWN";
    }
}

} // namespace ytui
