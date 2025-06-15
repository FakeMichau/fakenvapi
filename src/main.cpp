#include <unordered_map>

#include <dxgi.h>
#include <d3d11.h>
#include <windows.h>
#include <nvapi_interface.h>
#if _MSC_VER
#include <d3d12.h>
#else
#include "../external/d3d12.h"
#endif
#include <nvapi.h>
#include "fakenvapi.h"
#include "vulkan_hooks.h"
#include "../version.h"

#include "log.h"
#include "config.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        LowLatencyCtx::init();

        Config::get().init_config();
        if (Config::get().get_enable_logs())
            if (Config::get().get_enable_trace_logs())
                prepare_logging(spdlog::level::trace);
            else
                prepare_logging(spdlog::level::info);
        else
            prepare_logging(spdlog::level::off);

        spdlog::critical("fakenvapi version: {}", FAKENVAPI_VERSION);
        spdlog::info("Config enable_trace_logs: {}", Config::get().get_enable_trace_logs() ? "true" : "false");
        spdlog::info("Config force_latencyflex: {}", Config::get().get_force_latencyflex() ? "true" : "false");
        spdlog::info("Config force_reflex: {}", (int)Config::get().get_force_reflex());
        spdlog::info("Config lfx_mode: {}", (int)Config::get().get_latencyflex_mode());
        spdlog::info("Config save_pcl_to_file: {}", Config::get().get_save_pcl_to_file() ? "true" : "false");

        {
            // TODO: won't work for late loaded vulkan-1.dll
            auto vulkan_module = GetModuleHandleA("vulkan-1.dll");
            VulkanHooks::hook_vulkan(vulkan_module);
        }

        break;
    case DLL_PROCESS_DETACH:
        LowLatencyCtx::get()->deinit_current_tech();
        close_logging();
        LowLatencyCtx::shutdown();
        break;
    }
    return TRUE;
}

// names from: https://github.com/SveSop/nvapi_standalone/blob/master/dlls/nvapi/nvapi.c
NVAPI_INTERFACE_TABLE additional_interface_table[] = {
    { "NvAPI_Diag_ReportCallStart", 0x33c7358c },
    { "NvAPI_Diag_ReportCallReturn", 0x593e8644 },
    { "NvAPI_Unknown_1", 0xe9b009b9 },
    { "NvAPI_SK_1", 0x57f7caac },
    { "NvAPI_SK_2", 0x11104158 },
    { "NvAPI_SK_3", 0xe3795199 },
    { "NvAPI_SK_4", 0xdf0dfcdd },
    { "NvAPI_SK_5", 0x932ac8fb }
};

namespace fakenvapi {
    extern "C" {
        NvAPI_Status __cdecl placeholder() {
            // return OK();
            // return ERROR(NVAPI_NO_IMPLEMENTATION);
            return NVAPI_NO_IMPLEMENTATION; // no logging
        }

        static std::unordered_map<NvU32, void*> registry;

        __declspec(dllexport) void* __cdecl nvapi_QueryInterface(NvU32 id) {
            auto entry = registry.find(id);
            if (entry != registry.end())
                return entry->second;

            constexpr auto original_size = sizeof(nvapi_interface_table)/sizeof(nvapi_interface_table[0]);
            constexpr auto additional_size = sizeof(additional_interface_table)/sizeof(additional_interface_table[0]);
            constexpr auto fakenvapi_size = sizeof(fakenvapi_interface_table)/sizeof(fakenvapi_interface_table[0]);

            constexpr auto total_size = original_size + additional_size + fakenvapi_size;

            struct NVAPI_INTERFACE_TABLE extended_interface_table[total_size] {};
            memcpy(extended_interface_table, nvapi_interface_table, sizeof(nvapi_interface_table)); // copy original table

            for (unsigned int i = 0; i < additional_size; i++) {
                extended_interface_table[original_size + i] = additional_interface_table[i];
            }

            for (unsigned int i = 0; i < fakenvapi_size; i++) {
                extended_interface_table[original_size + additional_size + i].func = fakenvapi_interface_table[i].func;
                extended_interface_table[original_size + additional_size + i].id = fakenvapi_interface_table[i].id;
            }

            auto it = std::find_if(
                std::begin(extended_interface_table),
                std::end(extended_interface_table),
                [id](const auto& item) { return item.id == id; });

            if (it == std::end(extended_interface_table)) {
                spdlog::debug("NvAPI_QueryInterface (0x{:x}): Unknown interface ID", id);
                return registry.insert({ id, nullptr }).first->second;
            }

            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Initialize)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetInterfaceVersionString)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumNvidiaDisplayHandle)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetLogicalGPUFromPhysicalGPU)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumPhysicalGPUs)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumLogicalGPUs)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetGPUIDfromPhysicalGPU)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetPhysicalGPUFromGPUID)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetPhysicalGPUsFromDisplay)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetPhysicalGPUsFromLogicalGPU)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetErrorMessage)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetDisplayDriverVersion)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetLogicalGpuInfo)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetConnectedDisplayIds)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_CudaEnumComputeCapableGpus)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetArchInfo)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetPCIIdentifiers)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetFullName)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetGpuCoreCount)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetAllClockFrequencies)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetAdapterIdFromPhysicalGpu)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetPstates20)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DISP_GetDisplayIdByDisplayName)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DISP_GetGDIPrimaryDisplayId)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Disp_SetOutputMode)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Disp_GetOutputMode)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Disp_GetHdrCapabilities)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Disp_HdrColorControl)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Mosaic_GetDisplayViewportsByResolution)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetDisplayDriverInfo)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetDriverAndBranchVersion)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetDisplayIdFromGpuAndOutputId)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetGpuAndOutputIdFromDisplayId)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_SetResourceHint)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_GetObjectHandleForResource)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_GetSleepStatus)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_GetLatency)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_SetSleepMode)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_SetLatencyMarker)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_Sleep)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_SetReflexSync)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_IsNvShaderExtnOpCodeSupported)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_BeginUAVOverlap)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_EndUAVOverlap)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_SetDepthBoundsTest)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetRaytracingCaps)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_IsNvShaderExtnOpCodeSupported)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_SetNvShaderExtnSlotSpaceLocalThread)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_NotifyOutOfBandCommandQueue)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_SetAsyncFrameMarker)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_InitLowLatencyDevice)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_DestroyLowLatencyDevice)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_GetSleepStatus)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_SetSleepMode)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_Sleep)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_GetLatency)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_SetLatencyMarker)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_NotifyOutOfBandVkQueue)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_CreateSession)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_LoadSettings)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_SaveSettings)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_GetBaseProfile)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_GetSetting)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_SetSetting)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_DestroySession)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Unknown_1)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SK_1)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SK_2)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SK_3)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SK_4)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SK_5)
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Unload)
            INSERT_AND_RETURN_WHEN_EQUALS(Fake_GetLatency)
            INSERT_AND_RETURN_WHEN_EQUALS(Fake_InformFGState)
            INSERT_AND_RETURN_WHEN_EQUALS(Fake_InformPresentFG)
            INSERT_AND_RETURN_WHEN_EQUALS(Fake_GetAntiLagCtx)
            INSERT_AND_RETURN_WHEN_EQUALS(Fake_GetLowLatencyCtx)

            spdlog::debug("{}: not implemented, placeholder given", it->func);
            return registry.insert({ id, (void*)placeholder }).first->second;
            // return registry.insert({ id, nullptr }).first->second;
        }
    }
}
