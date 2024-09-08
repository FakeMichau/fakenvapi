#pragma once

#include <dxgi.h>
#if defined __MINGW64__ || defined __MINGW32__
#include "../include/d3d12.h"
#else
#include <d3d12.h>
#endif

#include "../include/ffx_antilag2_dx12.h"
#include "../include/ffx_antilag2_dx11.h"

#include "../include/latencyflex.h"

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
    uint64_t latency = {};
    uint64_t frameTime = {};
    uint64_t target = {};
    uint64_t lfx_frame_id = {};
    bool needs_reset = false;
            
};

class LowLatency {
    AMD::AntiLag2DX12::Context context_dx12 = {};
    AMD::AntiLag2DX11::Context context_dx11 = {};
    unsigned long min_interval_us = 0;
    bool al_available = false;
    lfx::LatencyFleX *lf;

    static inline uint64_t GetTimestamp() {
        LARGE_INTEGER frequency;
        LARGE_INTEGER counter;

        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);
        return static_cast<uint64_t>(counter.QuadPart) * UINT64_C(1000000000) / static_cast<uint64_t>(frequency.QuadPart);
    }

public:
    CallSpot call_spot = SimulationStart;
    Mode mode = AntiLag2;
    LFXStats lfx_stats = {};
    bool fg = false;
    bool active = true;

    inline HRESULT init_al2(IUnknown *pDevice) {
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
        return 0x2137;
    }

    inline HRESULT update() {
        if (!active) return S_OK;

        if (al_available) mode = AntiLag2;
        else mode = LatencyFlex;

        log(std::format("LowLatency algo: {}", mode == AntiLag2 ? "AntiLag 2" : "LatencyFlex"));
        log(std::format("FG status: {}", fg ? "enabled" : "disabled"));

        if (mode == AntiLag2) {
            if (lfx_stats.lfx_frame_id != 1) lfx_stats.needs_reset = true;
            int max_fps = min_interval_us > 0 ? 1000000 / min_interval_us : 0;
            if (context_dx12.m_pAntiLagAPI)
                return AMD::AntiLag2DX12::Update(&context_dx12, true, max_fps);
            else
                return AMD::AntiLag2DX11::Update(&context_dx11, true, max_fps);
        } else if (mode == LatencyFlex) {
            if (lfx_stats.needs_reset) {
                log("LFX Reset");
                lfx_stats.lfx_frame_id = 1;
                lfx_stats.needs_reset = false;
                lf->Reset();
            }
            uint64_t currentTimestamp = GetTimestamp();
            uint64_t timestamp;

            // Set FPS Limiter
            lf->target_frame_time = 1000 * min_interval_us;

            lf->EndFrame(lfx_stats.lfx_frame_id, currentTimestamp, &lfx_stats.latency, &lfx_stats.frameTime);
            // log(std::format("LFX latency: {}", lfx_stats.latency));
            // log(std::format("LFX frameTime: {}", lfx_stats.frameTime));
            lfx_stats.lfx_frame_id++;
            lfx_stats.target = lf->GetWaitTarget(lfx_stats.lfx_frame_id);

            auto diff = lfx_stats.target - currentTimestamp;
            if (lfx_stats.target > currentTimestamp) {
                static uint64_t timeout_events = 0;
                uint64_t timeout_timestamp = currentTimestamp + 50000000ULL;
                if (lfx_stats.target > timeout_timestamp) {
                    timestamp = timeout_timestamp;
                    timeout_events++;
                    if (timeout_events > 5) lfx_stats.needs_reset = true;
                } else {
                    timestamp = lfx_stats.target;
                    timeout_events = 0;
                }
                std::this_thread::sleep_for(std::chrono::nanoseconds(diff));
            } else {
                timestamp = currentTimestamp;
            }

            lf->BeginFrame(lfx_stats.lfx_frame_id, lfx_stats.target, timestamp);
            return S_OK;
        }
        return S_FALSE;
    }

    inline HRESULT set_fg_type(bool interpolated) {
        return AMD::AntiLag2DX12::SetFrameGenFrameType(&context_dx12, interpolated);
    }

    inline HRESULT mark_end_of_rendering() {
        return AMD::AntiLag2DX12::MarkEndOfFrameRendering(&context_dx12);
    }

    inline void unload() {
        if (context_dx12.m_pAntiLagAPI) AMD::AntiLag2DX12::DeInitialize(&context_dx12);
        if (context_dx11.m_pAntiLagAPI) AMD::AntiLag2DX11::DeInitialize(&context_dx11);
    }

    void set_min_interval_us(unsigned long interval_us) {
        log(std::format("Max fps: {}", interval_us > 0 ? 1000000 / interval_us : 0));
        min_interval_us = interval_us;
    }

    void init_lfx() {
        lf = new lfx::LatencyFleX();
    }
};