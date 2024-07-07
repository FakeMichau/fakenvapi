#pragma once
#include <iostream>
#include <chrono>
#include <cstdint>
#include <string>
#include <source_location>
#include "../include/nvapi.h"

std::string getCurrentTimeFormatted();
void log(const std::string& log);
NvAPI_Status Ok(const std::source_location &location = std::source_location::current());
NvAPI_Status Error(NvAPI_Status status = NVAPI_ERROR, const std::source_location &location = std::source_location::current());
void prepareLogging(std::optional<std::string> fileName);
void closeLogging();
