#include "log.h"

NvAPI_Status Ok(const std::source_location& location) {
    std::cout << location.function_name() << ": OK" << std::endl;
    return NVAPI_OK;
}

NvAPI_Status Error(const std::source_location& location) {
    std::cout << location.function_name() << ": Error" << std::endl;
    return NVAPI_ERROR;
}

NvAPI_Status Error(NvAPI_Status status, const std::source_location& location) {
    std::cout << location.function_name() << ": Error" << std::endl;
    return status;
}

void log(std::string log) {
    std::cout << log << std::endl;
}