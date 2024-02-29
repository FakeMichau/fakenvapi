#include "log.h"
#include <fstream>

std::ofstream logFileStream;

NvAPI_Status Ok(const std::source_location& location) {
    log_time();
    logFileStream << location.function_name() << ": OK" << std::endl;
    return NVAPI_OK;
}

NvAPI_Status Error(const std::source_location& location) {
    log_time();
    logFileStream << location.function_name() << ": Error" << std::endl;
    return NVAPI_ERROR;
}

NvAPI_Status Error(NvAPI_Status status, const std::source_location& location) {
    log_time();
    logFileStream << location.function_name() << ": " << status << std::endl;
    return status;
}

void log_time() {
    logFileStream << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << ": ";
}

void log(std::string log) {
    log_time();
    logFileStream << log << std::endl;
}

void prepareLogging(std::string fileName) {
    logFileStream = std::ofstream(fileName, std::ios_base::out | std::ios_base::app);
}

void closeLogging() {
    logFileStream.close();
}