#include "ll_latencyflex.h"
#include "config.h"
#include "log.h"

void LatencyFlex::lfx_sleep(uint64_t reflex_frame_id) {
    if (!is_enabled())
        return;

    static LFXMode previous_lfx_mode = Config::get().get_latencyflex_mode();
    LFXMode lfx_mode = Config::get().get_latencyflex_mode();

    if (previous_lfx_mode != lfx_mode)
        needs_reset = true;

    previous_lfx_mode = lfx_mode;

    if (needs_reset) {
        spdlog::info("LFX Reset");
        eepy(200000000ULL);
        frame_id = 1;
        needs_reset = false;
        ctx->Reset();
    }

    uint64_t current_timestamp = get_timestamp();
    uint64_t timestamp;

    // Set FPS Limiter
    ctx->target_frame_time = 1000 * minimum_interval_us;

    if (lfx_mode == LFXMode::Conservative) lfx_end_frame(INVALID_ID); // it should not be using this frame id in the conservative mode

    mutex.lock();
    auto local_frame_id = lfx_mode == LFXMode::ReflexIDs ? reflex_frame_id : this->frame_id + 1;
    // log_event("lfx_get_wait_target", "{}", frame_id);
    target = ctx->GetWaitTarget(local_frame_id);
    mutex.unlock();

    if (target > current_timestamp) {
        static uint64_t timeout_events = 0;
        uint64_t timeout_timestamp = current_timestamp + 50000000ULL;
        if (target > timeout_timestamp) {
            // log_event("lfx_target_high", "{}", target - timeout_timestamp);
            timestamp = timeout_timestamp;
            timeout_events++;
            needs_reset = timeout_events > 5;
        } else {
            timestamp = target;
            timeout_events = 0;
        }
        // log_event("lfx_sleep", "{}", timestamp - current_timestamp);
        if (auto res = eepy(timestamp - current_timestamp); res)
            spdlog::error("Sleep command failed: {}", res);
    } else {
        timestamp = current_timestamp;
    }

    mutex.lock();
    this->frame_id++;
    // log_event("lfx_beginframe", "{}", frame_id);
    ctx->BeginFrame(local_frame_id, target, timestamp);
    mutex.unlock();
}

void LatencyFlex::lfx_end_frame(uint64_t reflex_frame_id) {
    auto current_timestamp = get_timestamp();
    mutex.lock();
    auto frame_id = Config::get().get_latencyflex_mode() == LFXMode::ReflexIDs ? reflex_frame_id : this->frame_id;
    // log_event("lfx_endframe", "{}", frame_id);
    ctx->EndFrame(frame_id, current_timestamp, &latency, &frame_time);
    mutex.unlock();
    spdlog::trace("LFX latency: {}, frame_time: {}, current_timestamp: {}", latency, frame_time, current_timestamp);
}

bool LatencyFlex::init(IUnknown *pDevice) {
    current_call_spot = CallSpot::SimulationStart;

    if (!ctx) {
        ctx = new lfx::LatencyFleX();
        spdlog::info("LatencyFleX initialized");
        return true;
    }

    return false;
};

void LatencyFlex::deinit() {
    if (ctx) {
        delete ctx;
        ctx = nullptr;
        spdlog::info("LatencyFlex deinitialized");
    }
};

void* LatencyFlex::get_tech_context() {
    return ctx;
};

void LatencyFlex::get_sleep_status(SleepParams *sleep_params)
{
    sleep_params->low_latency_enabled = is_enabled();
    sleep_params->fullscreen_vrr = true;
    sleep_params->control_panel_vsync_override = false;
};

void LatencyFlex::set_sleep_mode(SleepMode* sleep_mode) {
    // UNUSED:
    // low_latency_boost
    // use_markers_to_optimize

    low_latency_enabled = sleep_mode->low_latency_enabled;
    minimum_interval_us = sleep_mode->minimum_interval_us;
};

void LatencyFlex::sleep() {
    if (current_call_spot == CallSpot::SleepCall && Config::get().get_latencyflex_mode() != LFXMode::ReflexIDs)
        lfx_sleep(INVALID_ID);
};

void LatencyFlex::set_marker(MarkerParams* marker_params) {
    switch(marker_params->marker_type) {
        case MarkerType::SIMULATION_START:
            if (current_call_spot == CallSpot::SimulationStart)
                lfx_sleep(marker_params->frame_id);
        break;

        case MarkerType::RENDERSUBMIT_END:
            if (Config::get().get_latencyflex_mode() != LFXMode::Conservative)
                lfx_end_frame(marker_params->frame_id);
        break;
    }
};
