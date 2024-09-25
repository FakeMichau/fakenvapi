#pragma once
#include <iostream>
#include "util.h"
#include "../external/nvapi.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#define OK() Ok(__func__)
#undef ERROR
#define ERROR() Error(__func__)
#define ERROR_VALUE(status) Error(__func__, status)

NvAPI_Status Ok(const char* function_name);
NvAPI_Status Error(const char* function_name, NvAPI_Status status = NVAPI_ERROR);
void prepare_logging(spdlog::level::level_enum level);
void close_logging();
