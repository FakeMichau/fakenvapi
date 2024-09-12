#include "log.h"

inline std::string extractFunctionName(const std::string& signature) {
    size_t start = signature.find("nvd::");
    if (start == std::string::npos) return {};
    start += 5;
    size_t end = signature.find('(', start);
    if (end == std::string::npos) return {};

    return signature.substr(start, end - start);
}

NvAPI_Status Ok(const std::source_location& location) {
    spdlog::trace("{}: {}", extractFunctionName(location.function_name()), "OK");
    return NVAPI_OK;
}

NvAPI_Status Error(NvAPI_Status status, const std::source_location& location) {
    spdlog::trace("{}: {}", extractFunctionName(location.function_name()), fromErrorNr(status));
    return status;
}

void prepareLogging(spdlog::level::level_enum level) {
    try {
        if (level != spdlog::level::off) {
            auto logger = spdlog::basic_logger_mt("basic_logger", "fakenvapi.log");
            spdlog::set_default_logger(logger);
            if (level == spdlog::level::trace)
                spdlog::set_pattern("[%H:%M:%S.%f] [%L] [thread %t] %v");
            else
                spdlog::set_pattern("[%H:%M:%S.%f] [%L] %v");
            spdlog::set_level(level);
            spdlog::flush_on(level);
        }
    } catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
}

void closeLogging() {
    spdlog::default_logger()->flush();
	spdlog::shutdown();
}