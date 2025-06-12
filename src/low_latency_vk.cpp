#include "low_latency.h"

#include "low_latency_tech/ll_antilag_vk.h"
#include "low_latency_tech/ll_latencyflex.h"

#include "log.h"
#include "config.h"

// private
bool LowLatency::update_low_latency_tech(HANDLE vkDevice) {
    if (!currently_active_tech) {
        if (!Config::get().get_force_latencyflex()) {
            currently_active_tech = new AntiLagVk();
            if (currently_active_tech->init(nullptr)) {
                spdlog::info("LowLatency algo: AntiLag Vulkan");
                return true;
            }
            
            delete currently_active_tech;
        }

        currently_active_tech = new LatencyFlex();
        if (currently_active_tech->init(nullptr)) {
            spdlog::info("LowLatency algo: LatencyFlex");
            return true;
        }
    }
    
    static bool last_force_latencyflex = Config::get().get_force_latencyflex();
    bool force_latencyflex = Config::get().get_force_latencyflex();
    bool change_detected = last_force_latencyflex != force_latencyflex;
    last_force_latencyflex = force_latencyflex;
    
    if (change_detected) {
        if (deinit_current_tech()) {
            return update_low_latency_tech((HANDLE) nullptr); // call back to reinit
        } else {
            spdlog::error("Couldn't deinitialize low latency tech");
            return false;
        }
    }

    return true;
}

void LowLatency::get_latency_result(NV_VULKAN_LATENCY_RESULT_PARAMS* pGetLatencyParams) {
    for (auto i = 0; i < 64; i++) {
        // some data is sent into rsvd
        memcpy(&pGetLatencyParams->frameReport[i], &frame_reports[i], sizeof(frame_reports[i]));
    }
}

void LowLatency::add_marker_to_report(NV_VULKAN_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams) {
    auto current_timestamp = get_timestamp() / 1000;
    static auto last_sim_start = current_timestamp;
    static auto _2nd_last_sim_start = current_timestamp;
    auto current_report = &frame_reports[pSetLatencyMarkerParams->frameID % 64];
    current_report->frameID = pSetLatencyMarkerParams->frameID;
    current_report->gpuFrameTimeUs = (uint32_t)(last_sim_start - _2nd_last_sim_start);
    current_report->gpuActiveRenderTimeUs = 100;
    current_report->driverStartTime = current_timestamp;
    current_report->driverEndTime = current_timestamp + 100;
    current_report->gpuRenderStartTime = current_timestamp;
    current_report->gpuRenderEndTime = current_timestamp + 100;
    current_report->osRenderQueueStartTime = current_timestamp;
    current_report->osRenderQueueEndTime = current_timestamp + 100;
    switch (pSetLatencyMarkerParams->markerType) {
        case VULKAN_SIMULATION_START:
            _2nd_last_sim_start = last_sim_start;
            last_sim_start = get_timestamp() / 1000;
            current_report->simStartTime = last_sim_start;
            break;
        case VULKAN_SIMULATION_END:
            current_report->simEndTime = get_timestamp() / 1000;
            break;
        case VULKAN_RENDERSUBMIT_START:
            current_report->renderSubmitStartTime = get_timestamp() / 1000;
            break;
        case VULKAN_RENDERSUBMIT_END:
            current_report->renderSubmitEndTime = get_timestamp() / 1000;
            break;
        case VULKAN_PRESENT_START:
            current_report->presentStartTime = get_timestamp() / 1000;
            break;
        case VULKAN_PRESENT_END:
            current_report->presentEndTime = get_timestamp() / 1000;
            break;
        case VULKAN_INPUT_SAMPLE:
            current_report->inputSampleTime = get_timestamp() / 1000;
            break;
        default:
            break;
    }
}

// public
NvAPI_Status LowLatency::Sleep(HANDLE vkDevice) {
    if (!update_low_latency_tech(vkDevice))
        return ERROR();

    currently_active_tech->sleep();

    return OK();
}

NvAPI_Status LowLatency::SetSleepMode(HANDLE vkDevice, NV_VULKAN_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams) {
    if (!update_low_latency_tech(vkDevice))
        return ERROR();

    SleepMode sleep_mode{};

    sleep_mode.low_latency_enabled = pSetSleepModeParams->bLowLatencyMode;
    sleep_mode.low_latency_boost = pSetSleepModeParams->bLowLatencyBoost;
    sleep_mode.minimum_interval_us = pSetSleepModeParams->minimumIntervalUs;
    sleep_mode.use_markers_to_optimize = true;

    currently_active_tech->set_sleep_mode(&sleep_mode);

    return OK();
}

NvAPI_Status LowLatency::GetSleepStatus(HANDLE vkDevice, NV_VULKAN_GET_SLEEP_STATUS_PARAMS* pGetSleepStatusParams)
{
    if (!update_low_latency_tech(vkDevice))
        return ERROR();

    SleepParams sleep_params{};

    currently_active_tech->get_sleep_status(&sleep_params);

    pGetSleepStatusParams->bLowLatencyMode = sleep_params.low_latency_enabled;

    return OK();
}

NvAPI_Status LowLatency::SetLatencyMarker(HANDLE vkDevice, NV_VULKAN_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams) {
    if (!update_low_latency_tech(vkDevice))
        return ERROR();

    update_effective_fg_state();

    update_enabled_override();

    add_marker_to_report(pSetLatencyMarkerParams);

    MarkerParams marker_params{};

    marker_params.frame_id = pSetLatencyMarkerParams->frameID;
    marker_params.marker_type = (MarkerType) pSetLatencyMarkerParams->markerType; // requires enums to match

    // This cast is not ideal as it needs to be cast to VkDevice while knowing it's vulkan
    currently_active_tech->set_marker((IUnknown*)vkDevice, &marker_params);

    spdlog::trace("{}: {}", marker_to_name(pSetLatencyMarkerParams->markerType), pSetLatencyMarkerParams->frameID);

    return NVAPI_OK;
}

NvAPI_Status LowLatency::GetLatency(HANDLE vkDevice, NV_VULKAN_LATENCY_RESULT_PARAMS* pGetLatencyParams) {
    if (!update_low_latency_tech(vkDevice))
        return ERROR();

    get_latency_result(pGetLatencyParams);

    return OK();
}
