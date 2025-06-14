#include "ll_antilag2.h"

inline HRESULT AntiLag2::al2_sleep() {
    int max_fps = 0;

    // TODO: test more, config for this?

    // if (effective_fg_state && minimum_interval_us != 0) {
    //     static uint64_t previous_frame_time = 0;
    //     uint64_t current_time = get_timestamp();
    //     uint64_t frame_time = current_time - previous_frame_time;
    //     if (frame_time < 1000 * minimum_interval_us) {
    //         if (auto res = eepy(minimum_interval_us * 1000 - frame_time); res)
    //             spdlog::error("Sleep command failed: {}", res);
    //     }
    //     previous_frame_time = get_timestamp();
    // } else {
    //     max_fps = minimum_interval_us > 0 ? (int) std::round(1000000.0f / minimum_interval_us) : 0;
    // }
    max_fps = minimum_interval_us > 0 ? (int) std::round(1000000.0f / minimum_interval_us) : 0;

    HRESULT result = {};

    // auto pre_sleep = get_timestamp();

    if (dx12_ctx.m_pAntiLagAPI)
        result = AMD::AntiLag2DX12::Update(&dx12_ctx, is_enabled(), max_fps);
    else if (dx11_ctx.m_pAntiLagAPI)
        result = AMD::AntiLag2DX11::Update(&dx11_ctx, is_enabled(), max_fps);

    // log_event("al2_sleep", "{}", get_timestamp() - pre_sleep);

    return result;
}

void AntiLag2::set_fg_type(bool interpolated, uint64_t frame_id) {
    if (effective_fg_state) {
        // log_event("al2_set_fg_type", "{}", reflex_frame_id);
        AMD::AntiLag2DX12::SetFrameGenFrameType(&dx12_ctx, interpolated);
    }
}

bool AntiLag2::init(IUnknown* pDevice) {
    if (!dx12_ctx.m_pAntiLagAPI && !dx11_ctx.m_pAntiLagAPI) {
        ID3D12Device* device = nullptr;
        HRESULT hr = pDevice->QueryInterface(__uuidof(ID3D12Device), reinterpret_cast<void**>(&device));
        if (hr == S_OK) {
            HRESULT init_return = AMD::AntiLag2DX12::Initialize(&dx12_ctx, device);
            if (init_return == S_OK) {
                spdlog::info("AntiLag 2 DX12 initialized");
                return true;
            } else {
                spdlog::info("AntiLag 2 DX12 initialization failed");
            }
        } else {
            HRESULT init_return = AMD::AntiLag2DX11::Initialize(&dx11_ctx);
            if (init_return == S_OK) {
                spdlog::info("AntiLag 2 DX11 initialized");
                return true;
            } else {
                spdlog::info("AntiLag 2 DX11 initialization failed");
            }
        }
    } else {
        spdlog::warn("Initialization of AntiLag 2 was attempted while the context is not null");
    }

    return false;
}

void AntiLag2::deinit() {
    if (dx12_ctx.m_pAntiLagAPI && !AMD::AntiLag2DX12::DeInitialize(&dx12_ctx))
        spdlog::info("AntiLag 2 DX12 deinitialized");

    if (dx11_ctx.m_pAntiLagAPI && !AMD::AntiLag2DX11::DeInitialize(&dx11_ctx))
        spdlog::info("AntiLag 2 DX11 deinitialized");
}

void* AntiLag2::get_tech_context() {
    if (dx12_ctx.m_pAntiLagAPI)
        return &dx12_ctx;
    else if (dx11_ctx.m_pAntiLagAPI)
        return &dx11_ctx;

    return nullptr;
}

void AntiLag2::get_sleep_status(SleepParams* sleep_params) {
    sleep_params->low_latency_enabled = is_enabled();
    sleep_params->fullscreen_vrr = true;
    sleep_params->control_panel_vsync_override = false;
}

void AntiLag2::set_sleep_mode(SleepMode* sleep_mode) {
    // UNUSED:
    // low_latency_boost
    // use_markers_to_optimize

    low_latency_enabled = sleep_mode->low_latency_enabled;
    minimum_interval_us = sleep_mode->minimum_interval_us; // don't convert to fps due to fg fps limit fallback using intervals
}

void AntiLag2::sleep() {
    if (current_call_spot == CallSpot::SleepCall)
        al2_sleep();
}

void AntiLag2::set_marker(IUnknown* pDevice, MarkerParams* marker_params) {
    switch(marker_params->marker_type) {
        case MarkerType::SIMULATION_START:
            if (current_call_spot == CallSpot::SimulationStart)
                al2_sleep();
        break;

        case MarkerType::PRESENT_START:
            if (effective_fg_state) {
                // log_event("al2_end_of_rendering", "{}", reflex_frame_id);
                AMD::AntiLag2DX12::MarkEndOfFrameRendering(&dx12_ctx);
            }
        break;
    }
}

void AntiLag2::set_async_marker(MarkerParams* marker_params) {
    if (marker_params->marker_type == MarkerType::OUT_OF_BAND_PRESENT_START) {
        static uint64_t previous_frame_id = marker_params->frame_id;
        set_fg_type(previous_frame_id == marker_params->frame_id, marker_params->frame_id);
        previous_frame_id = marker_params->frame_id;
    }
}
