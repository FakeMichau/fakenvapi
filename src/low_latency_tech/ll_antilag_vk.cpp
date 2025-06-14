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
    max_fps = sleep_mode->minimum_interval_us > 0 ? (uint32_t) std::round(1000000.0f / sleep_mode->minimum_interval_us) : 0;
}

void AntiLagVk::sleep() {
    // This can be supported but prefer using markers with frame ids instead
    return;
}

void AntiLagVk::set_marker(IUnknown* pDevice, MarkerParams* marker_params) {
    auto mode = is_enabled() ? VK_ANTI_LAG_MODE_ON_AMD : VK_ANTI_LAG_MODE_OFF_AMD;

    static size_t call_count = 0;
    static size_t last_oob_present = 0;
    static bool using_oob_present = false;
    constexpr size_t allowed_gap = 10;

    call_count++;

    if (marker_params->marker_type == MarkerType::OUT_OF_BAND_PRESENT_START) {
        last_oob_present = call_count;
        if (!using_oob_present)
            using_oob_present = true;
    }

    if (using_oob_present && call_count - last_oob_present > allowed_gap)
        using_oob_present = false;
    
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
        antiLagDataInput.maxFPS = max_fps;

        if (VulkanHooks::o_vkAntiLagUpdateAMD)
        {
            spdlog::trace("AntiLag Input: {}, status: {}", marker_params->frame_id, is_enabled());

            VulkanHooks::o_vkAntiLagUpdateAMD((VkDevice)pDevice, &antiLagDataInput);
        }
    }

    if ((marker_params->marker_type == MarkerType::PRESENT_START && !using_oob_present) ||
        marker_params->marker_type == MarkerType::OUT_OF_BAND_PRESENT_START)
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
        antiLagDataPresent.maxFPS = max_fps;

        if (VulkanHooks::o_vkAntiLagUpdateAMD)
        {
            spdlog::trace("AntiLag Present: {}, status: {}", marker_params->frame_id, is_enabled());

            VulkanHooks::o_vkAntiLagUpdateAMD((VkDevice)pDevice, &antiLagDataPresent);
        }
    }
}
