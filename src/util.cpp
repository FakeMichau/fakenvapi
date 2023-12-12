#include "util.h"

void tonvss(NvAPI_ShortString nvss, std::string str) {
    str.resize(NVAPI_SHORT_STRING_MAX - 1);
    strcpy(nvss, str.c_str());
}