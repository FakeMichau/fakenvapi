#pragma once
#include <iostream>
#include "util.h"
#include "../external/nvapi.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#define OK() Ok(__FUNCTION__)
#undef ERROR
#define ERROR(...) Error(__FUNCTION__, ##__VA_ARGS__)

NvAPI_Status Ok(const char* function_name);
NvAPI_Status Error(const char* function_name, NvAPI_Status status = NVAPI_ERROR);
void prepareLogging(spdlog::level::level_enum level);
void closeLogging();
