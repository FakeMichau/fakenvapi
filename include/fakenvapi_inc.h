// Version 1.3.0

#pragma once

#include <nvapi.h>
#include <cstdint>

enum class Mode {
    AntiLag2,
    LatencyFlex,
    XeLL,
    AntiLagVk
};

NvAPI_Status __cdecl Fake_InformFGState(bool fg_state);

NvAPI_Status __cdecl Fake_InformPresentFG(bool frame_interpolated, uint64_t reflex_frame_id);

// Deprecated
NvAPI_Status __cdecl Fake_GetAntiLagCtx(void** antilag2_context);

NvAPI_Status __cdecl Fake_GetLowLatencyCtx(void** low_latency_context, Mode* mode);