#pragma once

#include "low_latency_tech.h"

#include <dxgi.h>
#if _MSC_VER
#include <d3d12.h>
#else
#include "../external/d3d12.h"
#endif

#include "util.h"

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

class LowLatency {
private:
    LowLatencyTech* currently_active_tech; // TODO: mutex the access???
    FrameReport frame_reports[64]{};
    std::optional<bool> forced_fg;
    bool fg;
    
    bool update_low_latency_tech(IUnknown* pDevice);
    void update_effective_fg_state();
    void report_marker(NV_LATENCY_MARKER_PARAMS *pSetLatencyMarkerParams);

public:
    LowLatency() = default;
    ~LowLatency() { deinit_current_tech(); };

    bool deinit_current_tech();
    void get_latency_result(NV_LATENCY_RESULT_PARAMS* pGetLatencyParams);
    void set_forced_fg(std::optional<bool> forced_fg) { this->forced_fg = forced_fg; }; // TODO: needs more work
    void set_fg_type(bool interpolated, uint64_t frame_id) { currently_active_tech->set_fg_type(interpolated, frame_id); }
    void get_low_latency_context(void** low_latency_context, Mode* low_latency_tech);

    NvAPI_Status GetSleepStatus(IUnknown* pDevice, NV_GET_SLEEP_STATUS_PARAMS* pGetSleepStatusParams);
    NvAPI_Status SetSleepMode(IUnknown* pDevice, NV_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams);
    NvAPI_Status Sleep(IUnknown* pDevice);
    NvAPI_Status SetLatencyMarker(IUnknown* pDev, NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams);
    NvAPI_Status SetAsyncFrameMarker(ID3D12CommandQueue* pCommandQueue, NV_ASYNC_FRAME_MARKER_PARAMS* pSetAsyncFrameMarkerParams);
    NvAPI_Status GetLatency(IUnknown* pDev, NV_LATENCY_RESULT_PARAMS* pGetLatencyParams);
};