#pragma once

#include <dxgi.h>
#if _MSC_VER
#include <d3d12.h>
#else
#include "../external/d3d12.h"
#endif

#if _WIN64
#include "../external/ffx_antilag2_dx12.h"
#include "../external/ffx_antilag2_dx11.h"
#endif

#include "../external/latencyflex.h"

#include "log.h"
#include "config.h"

enum class Mode {
    AntiLag2,
    LatencyFlex,
};

enum class CallSpot {
    SleepCall = 0,
    InputSample = 1,
    SimulationStart = 2
};

struct LFXStats {
    uint64_t latency = 0;
    uint64_t frame_time = 1;
    uint64_t target = 0;
    uint64_t frame_id = 0;
    bool needs_reset = false;      
};

struct FrameReport {
    NvU64 frameID;
    NvU64 inputSampleTime;
    NvU64 simStartTime;
    NvU64 simEndTime;
    NvU64 renderSubmitStartTime;
    NvU64 renderSubmitEndTime;
    NvU64 presentStartTime;
    NvU64 presentEndTime;
    NvU64 driverStartTime;
    NvU64 driverEndTime;
    NvU64 osRenderQueueStartTime;
    NvU64 osRenderQueueEndTime;
    NvU64 gpuRenderStartTime;
    NvU64 gpuRenderEndTime;
    NvU32 gpuActiveRenderTimeUs;
    NvU32 gpuFrameTimeUs;
    NvU8 rsvd[120];
};

class LowLatency {
#if _WIN64
    Mode mode = Mode::AntiLag2;
#else
    Mode mode = Mode::LatencyFlex;
#endif
    lfx::LatencyFleX *lfx_ctx = nullptr;
    std::mutex lfx_mutex;
    std::mutex update_mutex;
    unsigned long min_interval_us = 0;
    bool al_available = false;
    bool force_latencyflex = false;
    ForceReflex force_reflex = ForceReflex::InGame;

    static constexpr uint64_t pcl_max_inprogress_frames = 16;
    uint64_t pcl_start_timestamps[pcl_max_inprogress_frames] = {};
    uint64_t pcl_start_ids[pcl_max_inprogress_frames] = {};

    // https://learn.microsoft.com/en-us/windows/win32/sync/using-waitable-timer-objects
    static inline int timer_sleep(int64_t hundred_ns){
        static HANDLE timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        LARGE_INTEGER due_time;

        due_time.QuadPart = -hundred_ns;

        if(!timer)
            return 1;

        if (!SetWaitableTimerEx(timer, &due_time, 0, NULL, NULL, NULL, 0))
            return 2;

        if (WaitForSingleObject(timer, INFINITE) != WAIT_OBJECT_0)
            return 3;

        return 0;
    };

    static inline int busywait_sleep(int64_t ns) {
        auto current_time = get_timestamp();
        auto wait_until = current_time + ns;
        while (current_time < wait_until) {
            current_time = get_timestamp();
        }
        return 0;
    }

    inline int eepy(int64_t ns) {
        constexpr int64_t busywait_threshold = 2000000;
        int status {};
        auto current_time = get_timestamp();
        if (ns <= busywait_threshold)
            status = busywait_sleep(ns);
        else
            status = timer_sleep((ns - busywait_threshold) / 100);

        if (int64_t sleep_deviation = ns - (get_timestamp() - current_time); sleep_deviation > 0 && !status)
            status = busywait_sleep(sleep_deviation);

        return status;
    }

public:
#if _WIN64
    AMD::AntiLag2DX12::Context al2_dx12_ctx = {};
    AMD::AntiLag2DX11::Context al2_dx11_ctx = {};
#endif
    CallSpot call_spot = CallSpot::SleepCall;
    LFXStats lfx_stats = {};
    LFXMode lfx_mode = {};
    uint64_t calls_without_sleep = 0;
    FrameReport frame_reports[64];
    bool fg = false;
    bool forced_fg = false;
    bool active = true;

    inline void init_al2(IUnknown *pDevice) {
#if _WIN64
        if (mode == Mode::AntiLag2 && !al2_dx12_ctx.m_pAntiLagAPI && !al2_dx11_ctx.m_pAntiLagAPI) {
            ID3D12Device* device = nullptr;
            HRESULT hr = pDevice->QueryInterface(__uuidof(ID3D12Device), reinterpret_cast<void**>(&device));
            if (hr == S_OK) {
                HRESULT init_return = AMD::AntiLag2DX12::Initialize(&al2_dx12_ctx, device);
                if (al_available = init_return == S_OK; !al_available) {
                    mode = Mode::LatencyFlex;
                    spdlog::info("AntiLag 2 DX12 initialization failed");
                } else {
                    spdlog::info("AntiLag 2 DX12 initialized");
                }
            } else {
                HRESULT init_return = AMD::AntiLag2DX11::Initialize(&al2_dx11_ctx);
                if (al_available = init_return == S_OK; !al_available) {
                    mode = Mode::LatencyFlex;
                    spdlog::info("AntiLag 2 DX11 initialization failed");
                } else {
                    spdlog::info("AntiLag 2 DX11 initialized");
                }
            }
        }
#else
        al_available = false;
#endif
    }

    inline void update_config() {
        force_latencyflex = Config::get().get_force_latencyflex();
        force_reflex = Config::get().get_force_reflex();
        lfx_mode = Config::get().get_latencyflex_mode();
    }

    void init_lfx() {
        if (!lfx_ctx) {
            lfx_ctx = new lfx::LatencyFleX();
            update_config();
            spdlog::info("LatencyFleX initialized");
        }
    }

    inline HRESULT update(uint64_t reflex_frame_id) { 
        std::lock_guard<std::mutex> lock(update_mutex);

        update_config();

        log_event("update", "{}", reflex_frame_id);
        if (force_reflex == ForceReflex::ForceDisable || (force_reflex == ForceReflex::InGame && !active)) return S_FALSE;

        bool effective_fg_state = (fg || forced_fg);
        Mode previous_mode = mode;
        static bool previous_fg_status = effective_fg_state;
        static LFXMode previous_lfx_mode = lfx_mode;

        if (al_available && !force_latencyflex) 
            mode = Mode::AntiLag2;
        else 
            mode = Mode::LatencyFlex;

        if (previous_mode != mode) {
            spdlog::debug("Changed low latency algorithm to: {}", mode == Mode::AntiLag2 ? "AntiLag 2" : "LatencyFlex");
            if (mode == Mode::LatencyFlex)
                lfx_stats.needs_reset = true;
        }

        if (previous_fg_status != effective_fg_state) {
            spdlog::info("FG mode changed to: {}", effective_fg_state ? "enabled" : "disabled");
            lfx_stats.needs_reset = true;
        }
        previous_fg_status = effective_fg_state;

        if (previous_lfx_mode != lfx_mode)
            lfx_stats.needs_reset = true;
        previous_lfx_mode = lfx_mode;

        spdlog::debug("LowLatency algo: {}", mode == Mode::AntiLag2 ? "AntiLag 2" : "LatencyFlex");
        spdlog::debug("FG status: {}", effective_fg_state ? "enabled" : "disabled");

        if (mode == Mode::AntiLag2) {
#if _WIN64
            if (lfx_stats.frame_id != 1) lfx_stats.needs_reset = true;
            int max_fps = 0; 
            if ((fg || forced_fg) && min_interval_us != 0) {
                static uint64_t previous_frame_time = 0;
                uint64_t current_time = get_timestamp();
                uint64_t frame_time = current_time - previous_frame_time;
                if (frame_time < 1000 * min_interval_us) {
                    if (auto res = eepy(min_interval_us * 1000 - frame_time); res)
                        spdlog::error("Sleep command failed: {}", res);
                }
                previous_frame_time = get_timestamp();
            } else {
                max_fps = min_interval_us > 0 ? 1000000 / min_interval_us : 0;
            }
            HRESULT result = {};
            auto pre_sleep = get_timestamp();
            if (al2_dx12_ctx.m_pAntiLagAPI)
                result = AMD::AntiLag2DX12::Update(&al2_dx12_ctx, true, max_fps);
            else if (al2_dx11_ctx.m_pAntiLagAPI)
                result = AMD::AntiLag2DX11::Update(&al2_dx11_ctx, true, max_fps);
            log_event("al2_sleep", "{}", get_timestamp() - pre_sleep);
            return result;
#endif
        } else if (mode == Mode::LatencyFlex) {
            if (lfx_stats.needs_reset) {
                spdlog::info("LFX Reset");
                eepy(200000000ULL);
                lfx_stats.frame_id = 1;
                lfx_stats.needs_reset = false;
                lfx_ctx->Reset();
            }
            uint64_t current_timestamp = get_timestamp();
            uint64_t timestamp;

            // Set FPS Limiter
            lfx_ctx->target_frame_time = 1000 * min_interval_us;

            if (lfx_mode == LFXMode::Conservative) lfx_end_frame(0); // it should not be using this frame id in the conservative mode

            lfx_mutex.lock();
            auto frame_id = lfx_mode == LFXMode::ReflexIDs ? reflex_frame_id : lfx_stats.frame_id + 1;
            log_event("lfx_get_wait_target", "{}", frame_id);
            lfx_stats.target = lfx_ctx->GetWaitTarget(frame_id);
            lfx_mutex.unlock();

            if (lfx_stats.target > current_timestamp) {
                static uint64_t timeout_events = 0;
                uint64_t timeout_timestamp = current_timestamp + 50000000ULL;
                if (lfx_stats.target > timeout_timestamp) {
                    log_event("lfx_target_high", "{}", lfx_stats.target - timeout_timestamp);
                    timestamp = timeout_timestamp;
                    timeout_events++;
                    lfx_stats.needs_reset = timeout_events > 5;
                } else {
                    timestamp = lfx_stats.target;
                    timeout_events = 0;
                }
                log_event("lfx_sleep", "{}", timestamp - current_timestamp);
                if (auto res = eepy(timestamp - current_timestamp); res)
                    spdlog::error("Sleep command failed: {}", res);
            } else {
                timestamp = current_timestamp;
            }

            lfx_mutex.lock();
            lfx_stats.frame_id++;
            log_event("lfx_beginframe", "{}", frame_id);
            lfx_ctx->BeginFrame(frame_id, lfx_stats.target, timestamp);
            lfx_mutex.unlock();
            
            return S_OK;
        }
        return S_FALSE;
    }

    inline HRESULT set_fg_type(bool interpolated, uint64_t reflex_frame_id) {
#if _WIN64
        if (fg || forced_fg) {
            log_event("al2_set_fg_type", "{}", reflex_frame_id);
            return AMD::AntiLag2DX12::SetFrameGenFrameType(&al2_dx12_ctx, interpolated);
        }
#endif
        return S_FALSE;
    }

    inline HRESULT mark_end_of_rendering(uint64_t reflex_frame_id) {
#if _WIN64
        if (fg || forced_fg) {
            log_event("al2_end_of_rendering", "{}", reflex_frame_id);
            return AMD::AntiLag2DX12::MarkEndOfFrameRendering(&al2_dx12_ctx);
        }
#endif
        return S_FALSE;
    }

    inline void lfx_end_frame(uint64_t reflex_frame_id) {
        auto current_timestamp = get_timestamp();
        lfx_mutex.lock();
        auto frame_id = lfx_mode == LFXMode::ReflexIDs ? reflex_frame_id : lfx_stats.frame_id;
        log_event("lfx_endframe", "{}", frame_id);
        lfx_ctx->EndFrame(frame_id, current_timestamp, &lfx_stats.latency, &lfx_stats.frame_time);
        lfx_mutex.unlock();
        spdlog::debug("LFX latency: {}, frame_time: {}, current_timestamp: {}", lfx_stats.latency, lfx_stats.frame_time, current_timestamp);
    }

    inline void pcl_start(uint64_t reflex_frame_id) {
        pcl_start_ids[reflex_frame_id % pcl_max_inprogress_frames] = reflex_frame_id;
        pcl_start_timestamps[reflex_frame_id % pcl_max_inprogress_frames] = get_timestamp();
    }

    inline void pcl_end(uint64_t reflex_frame_id) {
        if (pcl_start_ids[reflex_frame_id % pcl_max_inprogress_frames] == reflex_frame_id) {
            pcl_start_ids[reflex_frame_id % pcl_max_inprogress_frames] = UINT64_MAX;
            double time_taken = get_timestamp() - pcl_start_timestamps[reflex_frame_id % pcl_max_inprogress_frames];
            double time_taken_ms = time_taken / 1000000;
            log_pcl(time_taken_ms);
        }
    }

    void report_marker(NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams) {
        auto current_timestamp = get_timestamp() / 1000;
        static auto last_sim_start = current_timestamp;
        static auto _2nd_last_sim_start = current_timestamp;
        auto current_report = &frame_reports[pSetLatencyMarkerParams->frameID % 64];
        current_report->frameID = pSetLatencyMarkerParams->frameID;
        current_report->gpuFrameTimeUs = last_sim_start - _2nd_last_sim_start;
        current_report->gpuActiveRenderTimeUs = 100;
        current_report->driverStartTime = current_timestamp;
        current_report->driverEndTime = current_timestamp + 100;
        current_report->gpuRenderStartTime = current_timestamp;
        current_report->gpuRenderEndTime = current_timestamp + 100;
        current_report->osRenderQueueStartTime = current_timestamp;
        current_report->osRenderQueueEndTime = current_timestamp + 100;
        switch (pSetLatencyMarkerParams->markerType) {
            case SIMULATION_START:
                _2nd_last_sim_start = last_sim_start;
                last_sim_start = get_timestamp() / 1000;
                current_report->simStartTime = last_sim_start;
                break;
            case SIMULATION_END:
                current_report->simEndTime = get_timestamp() / 1000;
                break;
            case RENDERSUBMIT_START:
                current_report->renderSubmitStartTime = get_timestamp() / 1000;
                break;
            case RENDERSUBMIT_END:
                current_report->renderSubmitEndTime = get_timestamp() / 1000;
                break;
            case PRESENT_START:
                current_report->presentStartTime = get_timestamp() / 1000;
                break;
            case PRESENT_END:
                current_report->presentEndTime = get_timestamp() / 1000;
                break;
            case INPUT_SAMPLE:
                current_report->inputSampleTime = get_timestamp() / 1000;
                break;
            default:
                break;
        }
    }

    inline void unload() {
        spdlog::info("Unloading lowlatency");
#if _WIN64
        if (al2_dx12_ctx.m_pAntiLagAPI && !AMD::AntiLag2DX12::DeInitialize(&al2_dx12_ctx))
            spdlog::info("AntiLag 2 DX12 deinitialized");
        if (al2_dx11_ctx.m_pAntiLagAPI && !AMD::AntiLag2DX11::DeInitialize(&al2_dx11_ctx))
            spdlog::info("AntiLag 2 DX11 deinitialized");
#endif
        if (lfx_ctx) {
            delete lfx_ctx;
            lfx_ctx = nullptr;
        }
        spdlog::info("LatencyFlex deinitialized");
    }

    void set_min_interval_us(unsigned long interval_us) {
        if (min_interval_us != interval_us) {
            min_interval_us = interval_us;
            spdlog::info("Changed max fps: {}", interval_us > 0 ? 1000000 / interval_us : 0);
        }
    }

    Mode get_mode() {
        return mode;
    }
};