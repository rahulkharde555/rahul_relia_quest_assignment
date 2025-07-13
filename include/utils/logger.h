#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

enum class LogLevel 
{
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger 
{
public:
    static void initialize(const std::string& log_file = "employee_api.log", LogLevel level = LogLevel::INFO);
    
    // Simple logging methods
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

private:
    static void log(LogLevel level, const std::string& message);
    
    // Thread-local storage for thread safety
    static thread_local std::ofstream thread_file_stream_;
    
    // Static members (shared across threads)
    static bool initialized_;
    static std::string log_file_;
    static LogLevel log_level_;
};

// Simple convenience macros that handle string concatenation
#define LOG_DEBUG(msg)    Logger::debug((msg))
#define LOG_INFO(msg)     Logger::info((msg))
#define LOG_WARN(msg)     Logger::warn((msg))
#define LOG_ERROR(msg)    Logger::error((msg)) 