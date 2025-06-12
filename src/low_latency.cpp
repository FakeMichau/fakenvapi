#include "low_latency.h"

// private
void LowLatency::update_effective_fg_state() {
    if (!currently_active_tech)
        return;

    if (forced_fg.has_value())
        currently_active_tech->set_effective_fg_state(forced_fg.value());
    else
        currently_active_tech->set_effective_fg_state(fg);
}

void LowLatency::update_enabled_override() {
    if (!currently_active_tech)
        return;

    currently_active_tech->set_low_latency_override(Config::get().get_force_reflex());
}

std::string LowLatency::marker_to_name(uint32_t marker) {
    switch (marker) {
        case SIMULATION_START:
            return "SIMULATION_START";
        break;
        case SIMULATION_END:
            return "SIMULATION_END";
        break;
        case RENDERSUBMIT_START:
            return "RENDERSUBMIT_START";
        break;
        case RENDERSUBMIT_END:
            return "RENDERSUBMIT_END";
        break;
        case PRESENT_START:
            return "PRESENT_START";
        break;
        case PRESENT_END:
            return "PRESENT_END";
        break;
        case INPUT_SAMPLE:
            return "INPUT_SAMPLE";
        break;
        case TRIGGER_FLASH:
            return "TRIGGER_FLASH";
        break;
        case PC_LATENCY_PING:
            return "PC_LATENCY_PING";
        break;
        case OUT_OF_BAND_RENDERSUBMIT_START:
            return "OUT_OF_BAND_RENDERSUBMIT_START";
        break;
        case OUT_OF_BAND_RENDERSUBMIT_END:
            return "OUT_OF_BAND_RENDERSUBMIT_END";
        break;
        case OUT_OF_BAND_PRESENT_START:
            return "OUT_OF_BAND_PRESENT_START";
        break;
        case OUT_OF_BAND_PRESENT_END:
            return "OUT_OF_BAND_PRESENT_END";
        break;
    }
    
    return "Unknown";
}

// public
bool LowLatency::deinit_current_tech() {
    if (currently_active_tech) {
        currently_active_tech->deinit();

        delete currently_active_tech;
        currently_active_tech = nullptr;

        return true;
    }

    return false;
}

void LowLatency::get_low_latency_context(void** low_latency_context, Mode* low_latency_tech) {
    if (!currently_active_tech || !low_latency_context)
        return;

    *low_latency_context = currently_active_tech->get_tech_context();
    *low_latency_tech = currently_active_tech->get_mode();
}