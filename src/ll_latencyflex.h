#pragma once

#include "low_latency_tech.h"
#include "../external/latencyflex.h"

class LatencyFlex : public virtual LowLatencyTech {
private:
    lfx::LatencyFleX *ctx = nullptr;

    std::mutex mutex;

    bool low_latency_enabled = false;
    uint32_t minimum_interval_us = 0;

    uint64_t latency = 0;
    uint64_t frame_time = 1;
    uint64_t target = 0;
    uint64_t frame_id = 0;
    bool needs_reset = false;
  
    void lfx_sleep(uint64_t frame_id);
    void lfx_end_frame(uint64_t frame_id);

public:
    LatencyFlex(): LowLatencyTech() {}

    // From LowLatencyTech
    bool init(IUnknown *pDevice) override;
    void deinit() override;

    Mode get_mode() override { return Mode::LatencyFlex; };
    void* get_tech_context() override;
    void set_fg_type(bool interpolated, uint64_t frame_id) override {}; // Not used by LFX
    void set_low_latency_override(ForceReflex low_latency_override) override { this->low_latency_override = low_latency_override; };
    void set_effective_fg_state(bool effective_fg_state) override { this->effective_fg_state = effective_fg_state; };

    bool is_enabled() override { return low_latency_override != ForceReflex::InGame ? low_latency_override == ForceReflex::ForceEnable : low_latency_enabled; };

    void get_sleep_status(SleepParams* sleep_params) override;
    void set_sleep_mode(SleepMode* sleep_mode) override;
    void sleep() override;
    void set_marker(MarkerParams* marker_params) override;
    void set_async_marker(MarkerParams* marker_params) override {}; // Not used by LFX
};