#pragma once
#include <iostream>
#include <chrono>
#include <cstdint>
#include <string>
#include <fstream>
#include <format>
#include <source_location>
#include "util.h"
#include "../external/nvapi.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

NvAPI_Status Ok(const std::source_location &location = std::source_location::current());
NvAPI_Status Error(NvAPI_Status status = NVAPI_ERROR, const std::source_location &location = std::source_location::current());
void prepareLogging(spdlog::level::level_enum level);
void closeLogging();
