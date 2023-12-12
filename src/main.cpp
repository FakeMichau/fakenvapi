#include <algorithm>
#include <unordered_map>
#include <format>

#include <dxgi.h>
#include "../include/nvapi_interface.h"
#include "../include/d3d12.h"
#include "../include/nvapi.h"

#include "log.h"

namespace nvd {
    extern "C" {
        NvAPI_Status __cdecl placeholder() {
            return Ok();
        }

        static std::unordered_map<NvU32, void*> registry;

        void* nvapi_QueryInterface(NvU32 id) {
            auto entry = registry.find(id);
            if (entry != registry.end())
                return entry->second;

            auto it = std::find_if(
                std::begin(nvapi_interface_table),
                std::end(nvapi_interface_table),
                [id](const auto& item) { return item.id == id; });

            if (it == std::end(nvapi_interface_table)) {
                log(std::format("NvAPI_QueryInterface (0x{:x}): Unknown interface ID", id));
                return registry.insert({ id, nullptr }).first->second;
            }

#define INSERT_AND_RETURN_WHEN_EQUALS(method) \
    if (std::string(it->func) == #method)     \
        return registry.insert({id, (void *)method}).first->second;
            INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Initialize)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetInterfaceVersionString)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumNvidiaDisplayHandle)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetLogicalGPUFromPhysicalGPU)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumPhysicalGPUs)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetConnectedDisplayIds)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_CudaEnumComputeCapableGpus)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetArchInfo)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetPCIIdentifiers)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetFullName)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetGpuCoreCount)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetAllClockFrequencies)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DISP_GetDisplayIdByDisplayName)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Mosaic_GetDisplayViewportsByResolution)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetDisplayDriverInfo)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetDriverAndBranchVersion)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetRaytracingCaps)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_IsNvShaderExtnOpCodeSupported)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_SetNvShaderExtnSlotSpaceLocalThread)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_CreateSession)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_LoadSettings)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_GetBaseProfile)
                INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Unload)

#undef INSERT_AND_RETURN_WHEN_EQUALS

                log(std::format("{}: not implemented, placeholder given", it->func));
            return registry.insert({ id, (void*)placeholder }).first->second;
            // return registry.insert({ id, nullptr }).first->second;
        }
    }
}
