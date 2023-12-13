#include "log.h"
#include <fstream>

std::ofstream logFileStream;

NvAPI_Status Ok(const std::source_location& location) {
    logFileStream << location.function_name() << ": OK" << std::endl;
    return NVAPI_OK;
}

NvAPI_Status Error(const std::source_location& location) {
    logFileStream << location.function_name() << ": Error" << std::endl;
    return NVAPI_ERROR;
}

NvAPI_Status Error(NvAPI_Status status, const std::source_location& location) {
    logFileStream << location.function_name() << ": " << status << std::endl;
    return status;
}

void log(std::string log) {
    logFileStream << log << std::endl;
}

void prepareLogging(std::string fileName) {
    logFileStream = std::ofstream(fileName, std::ios_base::out | std::ios_base::app);
}

void closeLogging() {
    logFileStream.close();
}