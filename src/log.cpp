#include "log.h"

NvAPI_Status Ok(const char* function_name) {
    spdlog::trace("{}: {}", function_name, "OK");
    return NVAPI_OK;
}

NvAPI_Status Error(const char* function_name, NvAPI_Status status) {
    spdlog::trace("{}: {}", function_name, from_error_nr(status));
    return status;
}

void prepare_logging(spdlog::level::level_enum level) {
    try {
        if (level != spdlog::level::off) {
            auto logger = spdlog::basic_logger_mt("basic_logger", "fakenvapi.log", true);
            spdlog::set_default_logger(std::move(logger));
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

void close_logging() {
    spdlog::default_logger()->flush();
	spdlog::shutdown();
}

void log_pcl(double pcl) {
    if (Config::get().get_save_pcl_to_file()) {
        std::ofstream output("pcl", std::ofstream::trunc);
        output << std::setprecision(2) << std::fixed << pcl;
        output.close();
    }
}