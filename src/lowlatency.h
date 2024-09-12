#pragma once

#include <chrono>

#include <dxgi.h>
#if defined __MINGW64__ || defined __MINGW32__
#include "../external/d3d12.h"
#else
#include <d3d12.h>
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

struct LFXStats {
    uint64_t latency = 0;
    uint64_t frame_time = 1;
    uint64_t target = 0;
    uint64_t frame_id = 0;
    bool needs_reset = false;      
};

class LowLatency {
#if _MSC_VER && _WIN64
    AMD::AntiLag2DX12::Context context_dx12 = {};
    AMD::AntiLag2DX11::Context context_dx11 = {};
    Mode mode = AntiLag2;
#else
    Mode mode = LatencyFlex;
#endif
    lfx::LatencyFleX *lf;
    unsigned long min_interval_us = 0;
    bool al_available = false;

    static inline uint64_t GetTimestamp() {
        LARGE_INTEGER frequency;
        LARGE_INTEGER counter;

        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);
        return static_cast<uint64_t>(counter.QuadPart) * UINT64_C(1000000000) / static_cast<uint64_t>(frequency.QuadPart);
    }

public:
    CallSpot call_spot = SimulationStart;
    LFXStats lfx_stats = {};
    bool fg = false;
    bool active = true;
    bool force_lfx = false;

    inline HRESULT init_al2(IUnknown *pDevice) {
#if _MSC_VER && _WIN64
        if (mode == AntiLag2 && !context_dx12.m_pAntiLagAPI && !context_dx11.m_pAntiLagAPI) {
            ID3D12Device* device = nullptr;
            HRESULT hr = pDevice->QueryInterface(__uuidof(ID3D12Device), reinterpret_cast<void**>(&device));
            HRESULT init_return = S_FALSE;
            if (hr == S_OK) {
                init_return = AMD::AntiLag2DX12::Initialize(&context_dx12, device);
            } else {
                init_return = AMD::AntiLag2DX11::Initialize(&context_dx11);
            }
            al_available = init_return == S_OK;
        }
#else
        al_available = false;
#endif
        return 0x2137;
    }

    void init_lfx() {
        lf = new lfx::LatencyFleX();
    }

    inline HRESULT update() {
        if (!active) return S_FALSE;

        if (al_available && !force_lfx && (min_interval_us == 0 || !fg)) 
            mode = AntiLag2;
        else 
            mode = LatencyFlex;

        spdlog::debug("LowLatency algo: {}", mode == AntiLag2 ? "AntiLag 2" : "LatencyFlex");
        spdlog::debug("FG status: {}", fg ? "enabled" : "disabled");

        if (mode == AntiLag2) {
#if _MSC_VER && _WIN64
            if (lfx_stats.frame_id != 1) lfx_stats.needs_reset = true;
            int max_fps = min_interval_us > 0 ? 1000000 / min_interval_us : 0;
            if (context_dx12.m_pAntiLagAPI)
                return AMD::AntiLag2DX12::Update(&context_dx12, true, max_fps);
            else if (context_dx11.m_pAntiLagAPI)
                return AMD::AntiLag2DX11::Update(&context_dx11, true, max_fps);
#endif
        } else if (mode == LatencyFlex) {
            if (lfx_stats.needs_reset) {
                spdlog::info("LFX Reset");
                lfx_stats.frame_id = 1;
                lfx_stats.needs_reset = false;
                lf->Reset();
            }
            uint64_t current_timestamp = GetTimestamp();
            uint64_t timestamp;

            // Set FPS Limiter
            lf->target_frame_time = 1000 * min_interval_us;

            lf->EndFrame(lfx_stats.frame_id, current_timestamp, &lfx_stats.latency, &lfx_stats.frame_time);
            spdlog::debug("LFX latency: {}, frame_time: {}, current_timestamp: {}", lfx_stats.latency, lfx_stats.frame_time, current_timestamp);
            lfx_stats.frame_id++;
            lfx_stats.target = lf->GetWaitTarget(lfx_stats.frame_id);

            if (lfx_stats.target > current_timestamp) {
                uint64_t extra_delay = 0;
                // uint64_t extra_delay = (lfx_stats.target - current_timestamp) * 0.15;
                static uint64_t timeout_events = 0;
                uint64_t timeout_timestamp = current_timestamp + 50000000ULL;
                if (lfx_stats.target > timeout_timestamp) {
                    timestamp = timeout_timestamp;
                    timeout_events++;
                    lfx_stats.needs_reset = timeout_events > 5;
                } else {
                    timestamp = lfx_stats.target + extra_delay;
                    timeout_events = 0;
                }
                std::this_thread::sleep_for(std::chrono::nanoseconds(lfx_stats.target + extra_delay - current_timestamp));
            } else {
                timestamp = current_timestamp;
            }

            lf->BeginFrame(lfx_stats.frame_id, lfx_stats.target, timestamp);
            return S_OK;
        }
        return S_FALSE;
    }

    inline HRESULT set_fg_type(bool interpolated) {
#if _MSC_VER && _WIN64
        if (mode == AntiLag2)
            return AMD::AntiLag2DX12::SetFrameGenFrameType(&context_dx12, interpolated);
#endif
        return S_FALSE;
    }

    inline HRESULT mark_end_of_rendering() {
#if _MSC_VER && _WIN64
        if (mode == AntiLag2)
            return AMD::AntiLag2DX12::MarkEndOfFrameRendering(&context_dx12);
#endif
        return S_FALSE;
    }

    inline void unload() {
#if _MSC_VER && _WIN64
        if (context_dx12.m_pAntiLagAPI) AMD::AntiLag2DX12::DeInitialize(&context_dx12);
        if (context_dx11.m_pAntiLagAPI) AMD::AntiLag2DX11::DeInitialize(&context_dx11);
#endif
    }

    void set_min_interval_us(unsigned long interval_us) {
        if (min_interval_us != interval_us) {
            min_interval_us = interval_us;
            spdlog::info("Changed max fps: {}", interval_us > 0 ? 1000000 / interval_us : 0);
        }
    }

    Mode get_mode() {
        return mode;
    }
};