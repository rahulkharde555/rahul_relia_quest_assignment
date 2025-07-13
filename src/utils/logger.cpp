#include "utils/logger.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

bool Logger::initialized_ = false;
std::string Logger::log_file_ = "employee_api.log";
LogLevel Logger::log_level_ = LogLevel::ERROR;
thread_local std::ofstream Logger::thread_file_stream_;

void Logger::log(LogLevel level, const std::string& message) 
{
    if (!initialized_) 
    {
        initialize();
    }
    
    if (level < log_level_) 
    {
        return;
    }
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    
    std::string level_str;
    switch (level) 
    {
        case LogLevel::DEBUG: level_str = "DEBUG"; break;
        case LogLevel::INFO:  level_str = "INFO";  break;
        case LogLevel::WARN:  level_str = "WARN";  break;
        case LogLevel::ERROR: level_str = "ERROR"; break;
    }
    
    std::string log_entry = "[" + ss.str() + "] [" + level_str + "] " + message;
    
    std::cout << log_entry << std::endl;
    
    if (thread_file_stream_.is_open()) 
    {
        thread_file_stream_ << log_entry << std::endl;
        thread_file_stream_.flush();
    }
}

void Logger::initialize(const std::string& log_file, LogLevel level) 
{
    if (initialized_) 
    {
        return;
    }
    
    log_file_ = log_file;
    log_level_ = level;
    initialized_ = true;
    
    if (!thread_file_stream_.is_open()) 
    {
        thread_file_stream_.open(log_file, std::ios::app);
        if (!thread_file_stream_.is_open()) 
        {
            std::cerr << "Failed to open log file: " << log_file << std::endl;
        }
    }
    
    info("Thread-local logger initialized successfully. Log file: " + log_file);
}

void Logger::debug(const std::string& message) 
{
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) 
{
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message) 
{
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message) 
{
    log(LogLevel::ERROR, message);
} 