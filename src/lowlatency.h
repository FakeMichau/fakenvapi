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
    uint64_t latency {};
    uint64_t frameTime {};
    uint64_t target {};
};

class LowLatency {
    AMD::AntiLag2DX12::Context context_dx12 = {};
    AMD::AntiLag2DX11::Context context_dx11 = {};
    unsigned int max_fps = 0;
    bool al_available = false;
    lfx::LatencyFleX *lf;
    uint64_t lfx_frame_id = 0;

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

        if (!fg && al_available) mode = AntiLag2;
        else mode = LatencyFlex;

        log(std::format("LowLatency algo: {}", mode == AntiLag2 ? "AntiLag 2" : "LatencyFlex"));
        log(std::format("FG status: {}", fg ? "enabled" : "disabled"));

        if (mode == AntiLag2) {
            lf->Reset();
            if (context_dx12.m_pAntiLagAPI)
                return AMD::AntiLag2DX12::Update(&context_dx12, true, max_fps);
            else
                return AMD::AntiLag2DX11::Update(&context_dx11, true, max_fps);
        } else if (mode == LatencyFlex) {
            uint64_t currentTimestamp = GetTimestamp();
            lf->target_frame_time = max_fps > 0 ? 1000000000U / max_fps : 0;
            if (lfx_frame_id == 0) {
                lfx_stats.target = lf->GetWaitTarget(lfx_frame_id);
                currentTimestamp -= 31526756;
                lf->BeginFrame(lfx_frame_id, lfx_stats.target, currentTimestamp);
            }

            lf->EndFrame(lfx_frame_id, currentTimestamp, &lfx_stats.latency, &lfx_stats.frameTime);
            log(std::format("LFX latency: {}", lfx_stats.latency));
            log(std::format("LFX frameTime: {}", lfx_stats.frameTime));
            lfx_frame_id++;
            lfx_stats.target = lf->GetWaitTarget(lfx_frame_id);

            auto diff = lfx_stats.target - currentTimestamp;
            if (lfx_stats.target > 0 && lfx_stats.target > currentTimestamp) {
                //if (lfx_frame_id > 100) {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(diff));
                //}
            }

            lf->BeginFrame(lfx_frame_id, lfx_stats.target, lfx_stats.target > currentTimestamp && lfx_frame_id >= 0 ? lfx_stats.target : currentTimestamp);
            return S_OK;
        }
        return S_FALSE;
    }

    void set_fps_limit(unsigned int fps) {
        log(std::format("Max fps: {}", fps));
        max_fps = fps;
    }

    void init_lfx() {
        lf = new lfx::LatencyFleX();
    }
};