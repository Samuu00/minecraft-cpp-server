#include "Logger.hpp"
#include <iostream>

std::mutex Logger::s_logMutex;

void Logger::init() {
    // Inizializzazione base
}

void Logger::writeLog(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(s_logMutex);
    std::string levelStr;
    switch(level) {
        case LogLevel::INFO: levelStr = "[INFO] "; break;
        case LogLevel::WARN: levelStr = "[WARN] "; break;
        case LogLevel::ERR:  levelStr = "[ERR]  "; break;
    }
    std::cout << levelStr << message << std::endl;
}
