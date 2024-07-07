#include "log.h"
#include <fstream>
#include <format>

std::ostream null(nullptr);
std::ostream* logStream = &null;
std::ofstream fileStream;

std::string getCurrentTimeFormatted() {
    auto now = std::chrono::system_clock::now();
    auto now_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::localtime(&now_t);
    auto now_duration = now - std::chrono::system_clock::from_time_t(std::mktime(&now_tm));
    auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(now_duration);
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << now_tm.tm_hour << ":"
        << std::setfill('0') << std::setw(2) << now_tm.tm_min << ":"
        << std::setfill('0') << std::setw(2) << now_tm.tm_sec << "."
        << std::setfill('0') << std::setw(6) << now_us.count();

    return oss.str();
}

void log(const std::string& log) {
    *logStream << "[" << getCurrentTimeFormatted() << "] " << log << std::endl;
}

NvAPI_Status Ok(const std::source_location& location) {
    log(std::format("{}: {}", location.function_name(), "OK"));
    return NVAPI_OK;
}

NvAPI_Status Error(NvAPI_Status status, const std::source_location& location) {
    log(std::format("{}: {}", location.function_name(), std::to_string(status)));
    return status;
}

void prepareLogging(std::optional<std::string> fileName) {
    if (fileName.has_value()) {
        fileStream.open(fileName.value(), std::ios_base::out | std::ios_base::app);
        if (fileStream.is_open()) {
            logStream = &fileStream;
            return;
        }
        else {
            std::cerr << "Failed to open log file: " << fileName.value() << std::endl;
        }
    }
    // logStream = &std::cout;
}

void closeLogging() {
    fileStream.close();
}