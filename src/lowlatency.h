#pragma once

#include <dxgi.h>
#if _MSC_VER
#include <d3d12.h>
#else
#include "../external/d3d12.h"
#endif

#if _MSC_VER && _WIN64
#include "../external/ffx_antilag2_dx12.h"
#include "../external/ffx_antilag2_dx11.h"
#endif

#include "../external/latencyflex.h"

#include "log.h"

enum Mode {
    AntiLag2,
    LatencyFlex,
};

enum CallSpot {
    SleepCall = 0,
    InputSample = 1,
    SimulationStart = 2
};

enum ForceReflex {
    InGame,
    ForceDisable,
    ForceEnable
};

enum LFXMode {
    Conservative,
    Aggressive,
    ReflexIDs
};

struct LFXStats {
    uint64_t latency = 0;
    uint64_t frame_time = 1;
    uint64_t target = 0;
    uint64_t frame_id = 0;
    bool needs_reset = false;      
};

class LowLatency {
#if _MSC_VER && _WIN64
    AMD::AntiLag2DX12::Context al2_dx12_ctx = {};
    AMD::AntiLag2DX11::Context al2_dx11_ctx = {};
    Mode mode = AntiLag2;
#else
    Mode mode = LatencyFlex;
#endif
    lfx::LatencyFleX *lfx_ctx = nullptr;
    std::mutex lfx_mutex;
    unsigned long min_interval_us = 0;
    bool al_available = false;
    bool force_latencyflex = false;
    bool double_markers = false;
    ForceReflex force_reflex = InGame;
    LFXMode lfx_mode = {};

    static inline uint64_t get_timestamp() {
        FILETIME fileTime;
        GetSystemTimePreciseAsFileTime(&fileTime);

        uint64_t time = (static_cast<uint64_t>(fileTime.dwHighDateTime) << 32) | fileTime.dwLowDateTime;

        return time * 100;
    }

    // https://learn.microsoft.com/en-us/windows/win32/sync/using-waitable-timer-objects
    static inline int timer_sleep(int64_t hundred_ns){
        static HANDLE timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        LARGE_INTEGER due_time;

        due_time.QuadPart = -hundred_ns;

        if(!timer)
            return 1;

        if (!SetWaitableTimerEx(timer, &due_time, 0, NULL, NULL, NULL, 0))
            return 2;

        if (WaitForSingleObject(timer, INFINITE) != WAIT_OBJECT_0)
            return 3;

        return 0;
    };

    static inline int busywait_sleep(int64_t ns) {
        auto current_time = get_timestamp();
        auto wait_until = current_time + ns;
        while (current_time < wait_until) {
            current_time = get_timestamp();
        }
        return 0;
    }

    inline int eepy(int64_t ns) {
        constexpr int64_t busywait_threshold = 2000000;
        int status {};
        auto current_time = get_timestamp();
        if (ns <= busywait_threshold)
            status = busywait_sleep(ns);
        else
            status = timer_sleep((ns - busywait_threshold) / 100);

        if (int64_t sleep_deviation = ns - (get_timestamp() - current_time); sleep_deviation > 0 && !status)
            status = busywait_sleep(sleep_deviation);

        return status;
    }

public:
    CallSpot call_spot = SimulationStart;
    LFXStats lfx_stats = {};
    uint64_t calls_without_sleep = 0;
    bool fg = false;
    bool active = true;

    inline void init_al2(IUnknown *pDevice) {
#if _MSC_VER && _WIN64
        if (mode == AntiLag2 && !al2_dx12_ctx.m_pAntiLagAPI && !al2_dx11_ctx.m_pAntiLagAPI) {
            ID3D12Device* device = nullptr;
            HRESULT hr = pDevice->QueryInterface(__uuidof(ID3D12Device), reinterpret_cast<void**>(&device));
            if (hr == S_OK) {
                HRESULT init_return = AMD::AntiLag2DX12::Initialize(&al2_dx12_ctx, device);
                if (al_available = init_return == S_OK; !al_available) {
                    mode = LatencyFlex;
                    spdlog::info("AntiLag 2 DX12 initialization failed");
                } else {
                    spdlog::info("AntiLag 2 DX12 initialized");
                }
            } else {
                HRESULT init_return = AMD::AntiLag2DX11::Initialize(&al2_dx11_ctx);
                if (al_available = init_return == S_OK; !al_available) {
                    mode = LatencyFlex;
                    spdlog::info("AntiLag 2 DX11 initialization failed");
                } else {
                    spdlog::info("AntiLag 2 DX11 initialized");
                }
            }
        }
#else
        al_available = false;
#endif
    }

    void init_lfx() {
        lfx_ctx = new lfx::LatencyFleX();
        force_latencyflex = get_config(L"fakenvapi", L"force_latencyflex", false);
        force_reflex = (ForceReflex)get_config(L"fakenvapi", L"force_reflex", 0);
        lfx_mode = (LFXMode)get_config(L"fakenvapi", L"latencyflex_mode", 0);
        spdlog::info("LatencyFleX initialized");
    }

    inline HRESULT update(uint64_t reflex_frame_id) { 
        if (force_reflex == ForceDisable || (force_reflex == InGame && !active)) return S_FALSE;

        Mode previous_mode = mode;
        static bool previous_fg_status = fg;

        if (al_available && !force_latencyflex) 
            mode = AntiLag2;
        else 
            mode = LatencyFlex;

        if (previous_mode != mode) {
            spdlog::debug("Changed low latency algorithm to: {}", mode == AntiLag2 ? "AntiLag 2" : "LatencyFlex");
            if (mode == LatencyFlex)
                lfx_stats.needs_reset = true;
        }
        if (previous_fg_status != fg) {
            spdlog::info("FG mode changed to: {}", fg ? "enabled" : "disabled");
            lfx_stats.needs_reset = true;
        }
        previous_fg_status = fg;

        spdlog::debug("LowLatency algo: {}", mode == AntiLag2 ? "AntiLag 2" : "LatencyFlex");
        spdlog::debug("FG status: {}", fg ? "enabled" : "disabled");

        if (mode == AntiLag2) {
#if _MSC_VER && _WIN64
            if (lfx_stats.frame_id != 1) lfx_stats.needs_reset = true;
            int max_fps = 0; 
            if (fg && min_interval_us != 0) {
                static uint64_t previous_frame_time = 0;
                uint64_t current_time = get_timestamp();
                uint64_t frame_time = current_time - previous_frame_time;
                if (frame_time < 1000 * min_interval_us) {
                    if (auto res = eepy(min_interval_us * 1000 - frame_time); res)
                        spdlog::error("Sleep command failed: {}", res);
                }
                previous_frame_time = get_timestamp();
            } else {
                max_fps = min_interval_us > 0 ? 1000000 / min_interval_us : 0;
            }
            if (al2_dx12_ctx.m_pAntiLagAPI)
                return AMD::AntiLag2DX12::Update(&al2_dx12_ctx, true, max_fps);
            else if (al2_dx11_ctx.m_pAntiLagAPI)
                return AMD::AntiLag2DX11::Update(&al2_dx11_ctx, true, max_fps);
#endif
        } else if (mode == LatencyFlex) {
            if (lfx_stats.needs_reset) {
                spdlog::info("LFX Reset");
                lfx_stats.frame_id = 1;
                lfx_stats.needs_reset = false;
                lfx_ctx->Reset();
            }
            uint64_t current_timestamp = get_timestamp();
            uint64_t timestamp;

            // Set FPS Limiter
            lfx_ctx->target_frame_time = 1000 * min_interval_us;

            if (lfx_mode == Conservative) lfx_end_frame(0); // it should not be using this frame id in the conservative mode

            lfx_mutex.lock();
            auto frame_id = lfx_mode == ReflexIDs ? reflex_frame_id : lfx_stats.frame_id + 1;
            lfx_stats.target = lfx_ctx->GetWaitTarget(frame_id);
            lfx_mutex.unlock();

            if (lfx_stats.target > current_timestamp) {
                static uint64_t timeout_events = 0;
                uint64_t timeout_timestamp = current_timestamp + 50000000ULL;
                if (lfx_stats.target > timeout_timestamp) {
                    timestamp = timeout_timestamp;
                    timeout_events++;
                    lfx_stats.needs_reset = timeout_events > 5;
                } else {
                    timestamp = lfx_stats.target;
                    timeout_events = 0;
                }
                if (auto res = eepy(timestamp - current_timestamp); res)
                    spdlog::error("Sleep command failed: {}", res);
            } else {
                timestamp = current_timestamp;
            }

            lfx_mutex.lock();
            lfx_stats.frame_id++;
            lfx_ctx->BeginFrame(frame_id, lfx_stats.target, timestamp);
            lfx_mutex.unlock();
            
            return S_OK;
        }
        return S_FALSE;
    }

    inline HRESULT set_fg_type(bool interpolated) {
#if _MSC_VER && _WIN64
        if (fg)
            return AMD::AntiLag2DX12::SetFrameGenFrameType(&al2_dx12_ctx, interpolated);
#endif
        return S_FALSE;
    }

    inline HRESULT mark_end_of_rendering() {
#if _MSC_VER && _WIN64
        if (fg)
            return AMD::AntiLag2DX12::MarkEndOfFrameRendering(&al2_dx12_ctx);
#endif
        return S_FALSE;
    }

    inline void lfx_end_frame(uint64_t reflex_frame_id) {
        if (mode == LatencyFlex) {
            auto current_timestamp = get_timestamp();
            lfx_mutex.lock();
            auto frame_id = lfx_mode == ReflexIDs ? reflex_frame_id : lfx_stats.frame_id;
            lfx_ctx->EndFrame(frame_id, current_timestamp, &lfx_stats.latency, &lfx_stats.frame_time);
            lfx_mutex.unlock();
            spdlog::debug("LFX latency: {}, frame_time: {}, current_timestamp: {}", lfx_stats.latency, lfx_stats.frame_time, current_timestamp);
        }
    }

    inline void unload() {
#if _MSC_VER && _WIN64
        if (al2_dx12_ctx.m_pAntiLagAPI && !AMD::AntiLag2DX12::DeInitialize(&al2_dx12_ctx))
            spdlog::info("AntiLag 2 DX12 deinitialized");
        if (al2_dx11_ctx.m_pAntiLagAPI && !AMD::AntiLag2DX11::DeInitialize(&al2_dx11_ctx))
            spdlog::info("AntiLag 2 DX11 deinitialized");
#endif
        free(lfx_ctx);
        spdlog::info("LatencyFlex deinitialized");
    }

    void set_min_interval_us(unsigned long interval_us) {
        if (min_interval_us != interval_us) {
            min_interval_us = interval_us;
            spdlog::info("Changed max fps: {}", interval_us > 0 ? 1000000 / interval_us : 0);
        }
    }

    LFXMode get_lfx_mode() {
        return lfx_mode;
    }

    Mode get_mode() {
        return mode;
    }

    bool is_double_markers() {
        return double_markers;
    }

    bool ignore_frameid(uint64_t frameid) {
        static constexpr uint64_t high_frameid_threshold = 1000000000;
        static bool low_frameid_encountered = false;
        if (frameid < high_frameid_threshold) {
            low_frameid_encountered = true;
            return false;
        } else {
            if (low_frameid_encountered) {
                if (!double_markers)
                    spdlog::warn("Double reflex latency markers detected, disable RTSS!");
                double_markers = true;
            }
            // For high frameids return true only if a low number has been seen before
            return low_frameid_encountered;
        }
    }
};