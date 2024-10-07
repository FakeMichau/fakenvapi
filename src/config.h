#pragma once
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

enum class ForceReflex {
    InGame,
    ForceDisable,
    ForceEnable
};

enum class LFXMode {
    Conservative,
    Aggressive,
    ReflexIDs
};

class Config {
    wchar_t path[MAX_PATH];

    bool _enable_logs = true;
    bool _enable_trace_logs = false;
    bool _force_latencyflex = false;
    LFXMode _latencyflex_mode = LFXMode::Conservative;
    ForceReflex _force_reflex = ForceReflex::InGame;

    std::mutex update;

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
        return GetPrivateProfileIntW(section, key, default_value, path);
    }

    void update_config() {
        update.lock();

        _enable_logs       = get_config(L"fakenvapi", L"enable_logs",       true);
        _enable_trace_logs = get_config(L"fakenvapi", L"enable_trace_logs", false);
        _force_latencyflex = get_config(L"fakenvapi", L"force_latencyflex", false);

        auto latencyflex_mode   = get_config(L"fakenvapi", L"latencyflex_mode",  (int)LFXMode::Conservative);
        auto force_reflex       = get_config(L"fakenvapi", L"force_reflex",      (int)ForceReflex::InGame);

        if (latencyflex_mode >= (int)LFXMode::Conservative && latencyflex_mode <= (int)LFXMode::ReflexIDs)
            _latencyflex_mode = (LFXMode)latencyflex_mode;
        else
            _latencyflex_mode = LFXMode::Conservative;

        if (force_reflex >= (int)ForceReflex::InGame && force_reflex <= (int)ForceReflex::ForceEnable)
            _force_reflex = (ForceReflex)force_reflex;
        else
            _force_reflex = ForceReflex::InGame;
        
        update.unlock();

        // Some of them won't be logged as logging needs to be set up
        spdlog::info("Config updated");
        spdlog::info("Config enable_trace_logs: {}", _enable_trace_logs ? "true" : "false");
        spdlog::info("Config force_latencyflex: {}", _force_latencyflex ? "true" : "false");
        spdlog::info("Config force_reflex: {}", (int)_force_reflex);
        spdlog::info("Config lfx_mode: {}", (int)_latencyflex_mode);
    }

    FILETIME get_last_write_time(const wchar_t file_path[MAX_PATH]) {
        WIN32_FILE_ATTRIBUTE_DATA file_info;
        if (GetFileAttributesExW(file_path, GetFileExInfoStandard, &file_info)) {
            return file_info.ftLastWriteTime;
        }
        FILETIME filetime = { 0, 0 };
        return filetime;
    }

    void monitor_config_file() {
        FILETIME last_write_time = get_last_write_time(path);

        wchar_t directory[MAX_PATH];
        std::memcpy(directory, path, MAX_PATH);
        wchar_t* last_slash = wcsrchr(directory, L'\\');
        if (last_slash != nullptr) {
            *last_slash = L'\0'; 
        }

        HANDLE change_handle = FindFirstChangeNotificationW(directory, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

        if (change_handle == INVALID_HANDLE_VALUE) {
            spdlog::error("Unable to set up file change notification.");
            return;
        }

        while (true) {
            if (DWORD wait_status = WaitForSingleObject(change_handle, INFINITE); wait_status == WAIT_OBJECT_0) {
                FILETIME current_write_time = get_last_write_time(path);

                if (CompareFileTime(&last_write_time, &current_write_time) != 0) {
                    update_config();
                    last_write_time = current_write_time;
                }

                if (FindNextChangeNotification(change_handle) == FALSE) {
                    spdlog::error("Unable to reset file change notification.");
                    break;
                }
            }
        }

        FindCloseChangeNotification(change_handle);
    }

public:
    static Config& get() {
        static Config instance;
        return instance;
    }

    void init_config() {
        get_ini_path(path);
        update_config();
        std::thread config_monitor_thread(&Config::monitor_config_file, this);
        config_monitor_thread.detach();
    }

    bool get_enable_logs () {
        return _enable_logs;
    }

    bool get_enable_trace_logs () {
        return _enable_trace_logs;
    }

    bool get_force_latencyflex () {
        return _force_latencyflex;
    }

    LFXMode get_latencyflex_mode() {
        return _latencyflex_mode;
    }

    ForceReflex get_force_reflex() {
        return _force_reflex;
    }
};
