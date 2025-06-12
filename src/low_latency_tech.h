#pragma once

#include <dxgi.h>
#if _MSC_VER
#include <d3d12.h>
#else
#include "../external/d3d12.h"
#endif

#include "log.h"

#define INVALID_ID 0xFFFFFFFFFFFFFFFF

enum class Mode {
    AntiLag2,
    LatencyFlex,
    XeLL,
    AntiLagVk
};

enum class CallSpot {
    SleepCall = 0,
    InputSample = 1,
    SimulationStart = 2
};

struct SleepParams {
    bool low_latency_enabled;
    bool fullscreen_vrr;
    bool control_panel_vsync_override;
};

struct SleepMode {
    bool low_latency_enabled;
    bool low_latency_boost;
    uint32_t minimum_interval_us; // 0 -> no fps limit
    bool use_markers_to_optimize; // TODO: log this if false
};

enum class MarkerType
{
    SIMULATION_START = 0,
    SIMULATION_END = 1,
    RENDERSUBMIT_START = 2,
    RENDERSUBMIT_END = 3,
    PRESENT_START = 4,
    PRESENT_END = 5,
    INPUT_SAMPLE = 6,
    TRIGGER_FLASH = 7,
    PC_LATENCY_PING = 8,
    OUT_OF_BAND_RENDERSUBMIT_START = 9,
    OUT_OF_BAND_RENDERSUBMIT_END = 10,
    OUT_OF_BAND_PRESENT_START = 11,
    OUT_OF_BAND_PRESENT_END = 12,
};

struct MarkerParams {
    uint64_t frame_id;
    MarkerType marker_type;
};

class LowLatencyTech {
protected:
    CallSpot current_call_spot = CallSpot::SleepCall;
    ForceReflex low_latency_override = ForceReflex::InGame;
    bool low_latency_enabled = false;
    bool effective_fg_state = false;

public:
    LowLatencyTech():
        current_call_spot(CallSpot::SleepCall), 
        low_latency_override(ForceReflex::InGame), 
        low_latency_enabled(false), 
        effective_fg_state(false) {}
    virtual ~LowLatencyTech() {}

    virtual bool init(IUnknown* pDevice) = 0;
    virtual void deinit() = 0;

    virtual Mode get_mode() = 0;
    virtual void* get_tech_context() = 0;
    virtual void set_fg_type(bool interpolated, uint64_t frame_id) = 0;
    virtual void set_low_latency_override(ForceReflex low_latency_override) = 0;
    virtual void set_effective_fg_state(bool effective_fg_state) = 0;

    virtual bool is_enabled() = 0;

    virtual void get_sleep_status(SleepParams* sleep_params) = 0;
    virtual void set_sleep_mode(SleepMode* sleep_mode) = 0;
    virtual void sleep() = 0;
    virtual void set_marker(IUnknown* pDevice, MarkerParams* marker_params) = 0;
    virtual void set_async_marker(MarkerParams* marker_params) = 0;
};