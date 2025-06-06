#pragma once

#include <dxgi.h>
#if _MSC_VER
#include <d3d12.h>
#else
#include "../external/d3d12.h"
#endif

#if _WIN64
#include "../external/ffx_antilag2_dx12.h"
#include "../external/ffx_antilag2_dx11.h"
#endif

#include "../external/latencyflex.h"
#include <xell_d3d12.h>

#include "log.h"
#include "config.h"

enum class Mode {
    AntiLag2,
    LatencyFlex,
    XeLL
};

enum class CallSpot {
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

struct FrameReport {
    NvU64 frameID;
    NvU64 inputSampleTime;
    NvU64 simStartTime;
    NvU64 simEndTime;
    NvU64 renderSubmitStartTime;
    NvU64 renderSubmitEndTime;
    NvU64 presentStartTime;
    NvU64 presentEndTime;
    NvU64 driverStartTime;
    NvU64 driverEndTime;
    NvU64 osRenderQueueStartTime;
    NvU64 osRenderQueueEndTime;
    NvU64 gpuRenderStartTime;
    NvU64 gpuRenderEndTime;
    NvU32 gpuActiveRenderTimeUs;
    NvU32 gpuFrameTimeUs;
    NvU8 rsvd[120];
};

#define INVALID_ID 0xFFFFFFFFFFFFFFFF

class LowLatencyOld {
#if _WIN64
    Mode mode = Mode::AntiLag2;
#else
    Mode mode = Mode::LatencyFlex;
#endif
    std::mutex lfx_mutex;
    std::mutex update_mutex;

    unsigned long min_interval_us = 0;
    ForceReflex force_reflex = ForceReflex::InGame;

    lfx::LatencyFleX *lfx_ctx = nullptr;
    bool xell_available = false;
    bool al_available = false;
    bool force_latencyflex = false;

    static constexpr uint64_t pcl_max_inprogress_frames = 16;
    uint64_t pcl_start_timestamps[pcl_max_inprogress_frames] = {};
    uint64_t pcl_start_ids[pcl_max_inprogress_frames] = {};

    static inline int timer_sleep(int64_t hundred_ns);
    static inline int busywait_sleep(int64_t ns);
    inline int eepy(int64_t ns);
    void report_marker(NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams);
    std::string get_algorithm_name();

public:
#if _WIN64
    AMD::AntiLag2DX12::Context al2_dx12_ctx = {};
    AMD::AntiLag2DX11::Context al2_dx11_ctx = {};
#endif
    xell_context_handle_t xell_ctx = nullptr;
    CallSpot call_spot = CallSpot::SleepCall;
    LFXStats lfx_stats = {};
    LFXMode lfx_mode = {};
    uint64_t calls_without_sleep = 0;
    FrameReport frame_reports[64]{};
    bool sent_sleep_frame_ids[64]{};
    bool fg = false;
    bool forced_fg = false;
    bool active = true;

    void init_al2(IUnknown *pDevice);
    void init_xell(IUnknown* pDevice);
    void init_lfx();
    inline void update_config();
    inline HRESULT update(uint64_t reflex_frame_id) ;
    HRESULT set_fg_type(bool interpolated, uint64_t reflex_frame_id);
    inline HRESULT mark_end_of_rendering(uint64_t reflex_frame_id);
    inline void lfx_end_frame(uint64_t reflex_frame_id);
    inline void pcl_start(uint64_t reflex_frame_id);
    inline void pcl_end(uint64_t reflex_frame_id);
    void xell_set_sleep(NV_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams);
    void handle_marker(NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams);
    void sleep_called();
    void unload();
    void set_min_interval_us(unsigned long interval_us);
    Mode get_mode();
};
