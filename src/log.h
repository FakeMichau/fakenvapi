#include <iostream>
#include <string>
#include <source_location>
#include "../include/nvapi.h"

void log(std::string log);
NvAPI_Status Ok(const std::source_location& location = std::source_location::current());
NvAPI_Status Error(const std::source_location& location = std::source_location::current());
NvAPI_Status Error(NvAPI_Status status, const std::source_location& location = std::source_location::current());