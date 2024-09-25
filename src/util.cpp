#include "util.h"

void tonvss(NvAPI_ShortString nvss, std::string str) {
    str.resize(NVAPI_SHORT_STRING_MAX - 1);
    strcpy(nvss, str.c_str());
}

static void get_ini_path(wchar_t* path) {
    HMODULE module = GetModuleHandle(NULL);
    GetModuleFileNameW(module, path, MAX_PATH);
    wchar_t* last_slash = wcsrchr(path, L'\\');
    if (last_slash != nullptr) {
        *last_slash = L'\0'; 
    }
    wcscat(path, L"\\fakenvapi.ini");
}

int get_config(const wchar_t* section, const wchar_t* key, int default_value) {
    static wchar_t path[MAX_PATH] = {};
    if (!path[0])
        get_ini_path(path);
    return GetPrivateProfileIntW(section, key, default_value, path);
}
