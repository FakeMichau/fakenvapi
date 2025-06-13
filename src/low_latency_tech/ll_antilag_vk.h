#pragma once

#include "low_latency_tech.h"

class AntiLagVk : public virtual LowLatencyTech {
private:
    uint32_t minimum_interval_us = 0;

public:
    AntiLagVk(): LowLatencyTech() {}

    // From LowLatencyTech
    bool init(IUnknown *pDevice) override;
    void deinit() override {}; // Not used by AntiLag VK

    Mode get_mode() override { return Mode::AntiLagVk; };
    void* get_tech_context() override;
    void set_fg_type(bool interpolated, uint64_t frame_id) override {}; // Not used by AntiLag VK
    void set_low_latency_override(ForceReflex low_latency_override) override { this->low_latency_override = low_latency_override; };
    void set_effective_fg_state(bool effective_fg_state) override { this->effective_fg_state = effective_fg_state; };

    bool is_enabled() override { return low_latency_override != ForceReflex::InGame ? low_latency_override == ForceReflex::ForceEnable : low_latency_enabled; };

    void get_sleep_status(SleepParams* sleep_params) override;
    void set_sleep_mode(SleepMode* sleep_mode) override;
    void sleep() override;
    void set_marker(IUnknown* pDevice, MarkerParams* marker_params) override;
    void set_async_marker(MarkerParams* marker_params) override {}; // Not used by AntiLag VK
};