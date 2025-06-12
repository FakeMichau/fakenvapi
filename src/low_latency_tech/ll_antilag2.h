#pragma once

#include "low_latency_tech.h"

#include "../external/ffx_antilag2_dx12.h"
#include "../external/ffx_antilag2_dx11.h"

class AntiLag2 : public virtual LowLatencyTech {
private:
    AMD::AntiLag2DX12::Context dx12_ctx = {};
    AMD::AntiLag2DX11::Context dx11_ctx = {};

    uint32_t minimum_interval_us = 0;

    HRESULT al2_sleep();

public:
    AntiLag2(): LowLatencyTech() {}

    // From LowLatencyTech
    bool init(IUnknown *pDevice) override;
    void deinit() override;

    Mode get_mode() override { return Mode::AntiLag2; };
    void* get_tech_context() override;
    void set_fg_type(bool interpolated, uint64_t frame_id) override;
    void set_low_latency_override(ForceReflex low_latency_override) override { this->low_latency_override = low_latency_override; };
    void set_effective_fg_state(bool effective_fg_state) override { this->effective_fg_state = effective_fg_state; };

    bool is_enabled() override { return low_latency_override != ForceReflex::InGame ? low_latency_override == ForceReflex::ForceEnable : low_latency_enabled; };

    void get_sleep_status(SleepParams* sleep_params) override;
    void set_sleep_mode(SleepMode* sleep_mode) override;
    void sleep() override;
    void set_marker(IUnknown* pDevice, MarkerParams* marker_params) override;
    void set_async_marker(MarkerParams* marker_params) override;
};