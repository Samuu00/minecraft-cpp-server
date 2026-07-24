#pragma once

#include <string>
#include <sstream>
#include <mutex>

enum class LogLevel {
    INFO,
    WARN,
    ERR
};

class Logger {
public:
    static void init();

    template<typename... Args>
    static void log(LogLevel level, Args&&... args) {
        std::ostringstream ss;
        (ss << ... << std::forward<Args>(args));
        writeLog(level, ss.str());
    }

private:
    static void writeLog(LogLevel level, const std::string& message);
    static std::mutex s_logMutex;
};

#define LOG_INFO(...) Logger::log(LogLevel::INFO, __VA_ARGS__)
#define LOG_WARN(...) Logger::log(LogLevel::WARN, __VA_ARGS__)
#define LOG_ERROR(...) Logger::log(LogLevel::ERR, __VA_ARGS__)
