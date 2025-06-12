#include "ll_antilag_vk.h"
#include "vulkan_hooks.h"

bool AntiLagVk::init(IUnknown* pDevice) {
    return VulkanHooks::o_vkAntiLagUpdateAMD != nullptr;
}

void* AntiLagVk::get_tech_context() {
    return nullptr;
}

void AntiLagVk::get_sleep_status(SleepParams* sleep_params) {
    sleep_params->low_latency_enabled = is_enabled();
    sleep_params->fullscreen_vrr = true;
    sleep_params->control_panel_vsync_override = false;
}

void AntiLagVk::set_sleep_mode(SleepMode* sleep_mode) {
    // UNUSED:
    // low_latency_boost
    // use_markers_to_optimize

    low_latency_enabled = sleep_mode->low_latency_enabled;
    minimum_interval_us = sleep_mode->minimum_interval_us; // don't convert to fps due to fg fps limit fallback using intervals
}

void AntiLagVk::sleep() {
    // This can be supported but prefer using markers with frame ids instead
    return;
}

void AntiLagVk::set_marker(IUnknown* pDevice, MarkerParams* marker_params) {
    auto mode = is_enabled() ? VK_ANTI_LAG_MODE_ON_AMD : VK_ANTI_LAG_MODE_OFF_AMD;
    auto max_fps = minimum_interval_us > 0 ? (uint32_t) std::round(1000000.0f / minimum_interval_us) : 0;
    
    if (marker_params->marker_type == MarkerType::SIMULATION_START)
    {
        // Before processing user input
        VkAntiLagPresentationInfoAMD inputInfo = {};
        inputInfo.sType = VK_STRUCTURE_TYPE_ANTI_LAG_PRESENTATION_INFO_AMD;
        inputInfo.stage = VK_ANTI_LAG_STAGE_INPUT_AMD;
        inputInfo.frameIndex = marker_params->frame_id;

        VkAntiLagDataAMD antiLagDataInput = {};
        antiLagDataInput.sType = VK_STRUCTURE_TYPE_ANTI_LAG_DATA_AMD;
        antiLagDataInput.mode = mode;
        antiLagDataInput.pPresentationInfo = &inputInfo;
        antiLagDataInput.maxFPS = max_fps; // Or your desired FPS cap

        if (VulkanHooks::o_vkAntiLagUpdateAMD)
        {
            spdlog::trace("AntiLag Input: {}, status: {}", marker_params->frame_id, is_enabled());

            VulkanHooks::o_vkAntiLagUpdateAMD((VkDevice)pDevice, &antiLagDataInput);
        }
    }

    if (marker_params->marker_type == MarkerType::PRESENT_START)
    {
        // Before calling vkQueuePresentKHR
        VkAntiLagPresentationInfoAMD presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_ANTI_LAG_PRESENTATION_INFO_AMD;
        presentInfo.stage = VK_ANTI_LAG_STAGE_PRESENT_AMD;
        presentInfo.frameIndex = marker_params->frame_id;

        VkAntiLagDataAMD antiLagDataPresent = {};
        antiLagDataPresent.sType = VK_STRUCTURE_TYPE_ANTI_LAG_DATA_AMD;
        antiLagDataPresent.mode = mode;
        antiLagDataPresent.pPresentationInfo = &presentInfo;
        antiLagDataPresent.maxFPS = max_fps; // Or your desired FPS cap

        if (VulkanHooks::o_vkAntiLagUpdateAMD)
        {
            spdlog::trace("AntiLag Present: {}, status: {}", marker_params->frame_id, is_enabled());

            VulkanHooks::o_vkAntiLagUpdateAMD((VkDevice)pDevice, &antiLagDataPresent);
        }
    }
}

void AntiLagVk::set_async_marker(MarkerParams* marker_params) {
    if (marker_params->marker_type == MarkerType::OUT_OF_BAND_PRESENT_START) {
        static uint64_t previous_frame_id = marker_params->frame_id;
        set_fg_type(previous_frame_id == marker_params->frame_id, marker_params->frame_id);
        previous_frame_id = marker_params->frame_id;
    }
}
