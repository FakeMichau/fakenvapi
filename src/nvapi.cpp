#include "nvapi.h"
#include <string>

namespace nvd {
    NvAPI_Status __cdecl NvAPI_Initialize() {
        IDXGIFactory1* pFactory = nullptr;
        if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory))) {
            log("Failed to create DXGI Factory");
            return Error();
        }

        IDXGIAdapter1* pAdapter = nullptr;
        if (FAILED(pFactory->EnumAdapters1(0, &pAdapter))) {
            log("Failed to enumerate adapters");
            pFactory->Release();
            return Error();
        }

        DXGI_ADAPTER_DESC1 adapterDesc;
        if (FAILED(pAdapter->GetDesc1(&adapterDesc))) {
            log("Failed to get adapter description");
            pAdapter->Release();
            pFactory->Release();
            return Error();
        }

        luid = adapterDesc.AdapterLuid;
        deviceId = adapterDesc.DeviceId;
        vendorId = adapterDesc.VendorId;
        subSysId = adapterDesc.SubSysId;
        revisionId = adapterDesc.Revision;

        pAdapter->Release();
        pFactory->Release();

        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GetInterfaceVersionString(NvAPI_ShortString desc) {
        tonvss(desc, "NvAPI dummy");
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS], NvU32* count) {
        handles[0] = nullptr;
        *count = 1;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_EnumLogicalGPUs(NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS], NvU32* count) {
        handles[0] = nullptr;
        *count = 1;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_EnumNvidiaDisplayHandle(NvU32 displayId, NvDisplayHandle* handle) {
        if (displayId == 0) {
            return Ok();
        }
        return Error(NVAPI_END_ENUMERATION);
    }

    NvAPI_Status __cdecl NvAPI_GetLogicalGPUFromPhysicalGPU(NvPhysicalGpuHandle physicalHandle, NvLogicalGpuHandle* logicalHandle) {
        *logicalHandle = nullptr;
        return Ok();
    }


    NvAPI_Status __cdecl NvAPI_GetGPUIDfromPhysicalGPU(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pGpuId) {
        *pGpuId = 42;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUFromGPUID(NvU32 gpuId, NvPhysicalGpuHandle* pPhysicalGPU) {
        *pPhysicalGPU = nullptr;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromDisplay(NvDisplayHandle hNvDisp, NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32* pGpuCount) {
        nvGPUHandle[0] = nullptr;
        *pGpuCount = 1;

        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GetErrorMessage(NvAPI_Status status, NvAPI_ShortString szMsg) {
        std::string error = fromErrorNr(status);
        log(std::format("NvAPI_GetErrorMessage gave this error: {}", error));
        tonvss(szMsg, error);
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GPU_CudaEnumComputeCapableGpus(NV_COMPUTE_GPU_TOPOLOGY* pComputeTopo) {
        auto pComputeTopoV1 = reinterpret_cast<NV_COMPUTE_GPU_TOPOLOGY_V1*>(pComputeTopo);
        pComputeTopoV1->gpuCount = 1;
        pComputeTopoV1->computeGpus[0].hPhysicalGpu = nullptr;
        pComputeTopoV1->computeGpus[0].flags = NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_CAPABLE | NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_ENABLE | NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_RECOMMENDED;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetConnectedDisplayIds(NvPhysicalGpuHandle handle, NV_GPU_DISPLAYIDS* displayIds, NvU32* displayCount, NvU32 flags) {
        *displayCount = 0;  // no displays connected, may cause issues
        // NVAPI_NVIDIA_DISPLAY_NOT_FOUND could be considered
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetArchInfo(NvPhysicalGpuHandle handle, NV_GPU_ARCH_INFO* archInfo) {
        archInfo->architecture = spoof::arch;
        archInfo->architecture_id = spoof::arch;
        archInfo->implementation = spoof::implementation;
        archInfo->implementation_id = spoof::implementation;
        archInfo->revision = spoof::revision;
        archInfo->revision_id = spoof::revision;

        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetLogicalGpuInfo(NvLogicalGpuHandle logicalHandle, NV_LOGICAL_GPU_DATA* logicalGpuData) {
        memcpy(logicalGpuData->pOSAdapterId, &luid, sizeof(luid));
        logicalGpuData->physicalGpuHandles[0] = nullptr;
        logicalGpuData->physicalGpuCount = 1;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pDeviceId, NvU32* pSubSystemId, NvU32* pRevisionId, NvU32* pExtDeviceId) {
        *pDeviceId = (deviceId << 16) | vendorId;
        *pSubSystemId = subSysId;
        *pRevisionId = revisionId;
        *pExtDeviceId = deviceId;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName) {
        tonvss(szName, spoof::fullGPUName);
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetGpuCoreCount(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pCount) {
        *pCount = 1;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetAllClockFrequencies(NvPhysicalGpuHandle hPhysicalGPU, NV_GPU_CLOCK_FREQUENCIES* pClkFreqs) {
        if (pClkFreqs == nullptr)
            return Error(NVAPI_INVALID_ARGUMENT);

        if (pClkFreqs->version != NV_GPU_CLOCK_FREQUENCIES_VER_1 && pClkFreqs->version != NV_GPU_CLOCK_FREQUENCIES_VER_2 && pClkFreqs->version != NV_GPU_CLOCK_FREQUENCIES_VER_3)
            return Error(NVAPI_INCOMPATIBLE_STRUCT_VERSION);

        if (pClkFreqs->ClockType != static_cast<unsigned int>(NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ)) {
            return Error(NVAPI_NOT_SUPPORTED);
        }

        // Reset all clock data for all domains
        for (auto& domain : pClkFreqs->domain) {
            domain.bIsPresent = 0;
            domain.frequency = 0;
        }

        unsigned int clock = 1600;

        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent = 1;
        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency = (clock * 1000);

        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent = 1;
        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency = (clock * 1000);

        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].bIsPresent = 1;
        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].frequency = (clock * 1000);
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetAdapterIdFromPhysicalGpu(NvPhysicalGpuHandle hPhysicalGpu, void* pOSAdapterId) {
        memcpy(pOSAdapterId, &luid, sizeof(luid));
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPstates20(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_PERF_PSTATES20_INFO* pPstatesInfo) {
        if (!pPstatesInfo)
            return Error();

        // Initialize the structure with mock data
        pPstatesInfo->version = NV_GPU_PERF_PSTATES20_INFO_VER;
        pPstatesInfo->numPstates = 2;  // Example: 2 P-states
        pPstatesInfo->numClocks = 3;
        pPstatesInfo->numBaseVoltages = 1;
        pPstatesInfo->ov.numVoltages = 0;
        
        // Fill mock data for P-state 0
        pPstatesInfo->pstates[0].pstateId = NVAPI_GPU_PERF_PSTATE_P0;
        pPstatesInfo->pstates[0].bIsEditable = false;
        pPstatesInfo->pstates[0].clocks[0].domainId = NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS;
        pPstatesInfo->pstates[0].clocks[0].freqDelta_kHz.value = 1000; // 1 MHz
        pPstatesInfo->pstates[0].clocks[0].freqDelta_kHz.valueRange.min = 800;
        pPstatesInfo->pstates[0].clocks[0].freqDelta_kHz.valueRange.max = 1200;
        pPstatesInfo->pstates[0].clocks[1].domainId = NVAPI_GPU_PUBLIC_CLOCK_MEMORY;
        pPstatesInfo->pstates[0].clocks[1].freqDelta_kHz.value = 1000; // 1 MHz
        pPstatesInfo->pstates[0].clocks[1].freqDelta_kHz.valueRange.min = 800;
        pPstatesInfo->pstates[0].clocks[1].freqDelta_kHz.valueRange.max = 1200;
        pPstatesInfo->pstates[0].clocks[2].domainId = NVAPI_GPU_PUBLIC_CLOCK_VIDEO;
        pPstatesInfo->pstates[0].clocks[2].freqDelta_kHz.value = 1000; // 1 MHz
        pPstatesInfo->pstates[0].clocks[2].freqDelta_kHz.valueRange.min = 800;
        pPstatesInfo->pstates[0].clocks[2].freqDelta_kHz.valueRange.max = 1200;
        pPstatesInfo->pstates[0].baseVoltages[0].volt_uV = 1000000; // 1V
        pPstatesInfo->pstates[0].baseVoltages[0].bIsEditable = false;
        pPstatesInfo->pstates[0].baseVoltages[0].voltDelta_uV.value = 1000;
        pPstatesInfo->pstates[0].baseVoltages[0].voltDelta_uV.valueRange.min = 0;
        pPstatesInfo->pstates[0].baseVoltages[0].voltDelta_uV.valueRange.max = 0;

        // Fill mock data for P-state 1
        pPstatesInfo->pstates[1].pstateId = NVAPI_GPU_PERF_PSTATE_P1;
        pPstatesInfo->pstates[1].bIsEditable = false;
        pPstatesInfo->pstates[1].clocks[0].domainId = NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS;
        pPstatesInfo->pstates[1].clocks[0].freqDelta_kHz.value = 1000; // 1 MHz
        pPstatesInfo->pstates[1].clocks[0].freqDelta_kHz.valueRange.min = 800;
        pPstatesInfo->pstates[1].clocks[0].freqDelta_kHz.valueRange.max = 1200;
        pPstatesInfo->pstates[1].clocks[1].domainId = NVAPI_GPU_PUBLIC_CLOCK_MEMORY;
        pPstatesInfo->pstates[1].clocks[1].freqDelta_kHz.value = 1000; // 1 MHz
        pPstatesInfo->pstates[1].clocks[1].freqDelta_kHz.valueRange.min = 800;
        pPstatesInfo->pstates[1].clocks[1].freqDelta_kHz.valueRange.max = 1200;
        pPstatesInfo->pstates[1].clocks[2].domainId = NVAPI_GPU_PUBLIC_CLOCK_VIDEO;
        pPstatesInfo->pstates[1].clocks[2].freqDelta_kHz.value = 1000; // 1 MHz
        pPstatesInfo->pstates[1].clocks[2].freqDelta_kHz.valueRange.min = 800;
        pPstatesInfo->pstates[1].clocks[2].freqDelta_kHz.valueRange.max = 1200;
        pPstatesInfo->pstates[1].baseVoltages[0].volt_uV = 1000000; // 1V
        pPstatesInfo->pstates[1].baseVoltages[0].bIsEditable = false;
        pPstatesInfo->pstates[1].baseVoltages[0].voltDelta_uV.value = 1000;
        pPstatesInfo->pstates[1].baseVoltages[0].voltDelta_uV.valueRange.min = 0;
        pPstatesInfo->pstates[1].baseVoltages[0].voltDelta_uV.valueRange.max = 0;

        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetDisplayIdByDisplayName(const char* displayName, NvU32* displayId) {
        *displayId = 0;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetGDIPrimaryDisplayId(NvU32* displayId) {
        *displayId = 0;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_Disp_SetOutputMode(NvU32 displayId, NV_DISPLAY_OUTPUT_MODE* pDisplayMode) {
        *pDisplayMode = NV_DISPLAY_OUTPUT_MODE_SDR; // meaning no HDR
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_Disp_GetOutputMode(NvU32 displayId, NV_DISPLAY_OUTPUT_MODE* pDisplayMode) {
        *pDisplayMode = NV_DISPLAY_OUTPUT_MODE_SDR; // meaning no HDR
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_Mosaic_GetDisplayViewportsByResolution(NvU32 displayId, NvU32 srcWidth, NvU32 srcHeight, NV_RECT viewports[NV_MOSAIC_MAX_DISPLAYS], NvU8* bezelCorrected) {
        for (int i = 0; i < NV_MOSAIC_MAX_DISPLAYS; i++) {
            viewports[i].top = 0;
            viewports[i].left = 0;
            viewports[i].right = 0;
            viewports[i].bottom = 0;
        }
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDisplayDriverInfo(NV_DISPLAY_DRIVER_INFO* driverInfo) {
        spoof::driverInfo(driverInfo);
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
        memset(pDriverVersion, 0, sizeof(NvU32));
        memcpy(pDriverVersion, &spoof::driverVersion, sizeof(NvU32));
        // *pDriverVersion = spoof::driverVersion;
        tonvss(szBuildBranchString, spoof::buildBranch);
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDisplayIdFromGpuAndOutputId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputId, NvU32* displayId) {
        *displayId = 0;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetGpuAndOutputIdFromDisplayId(NvU32 displayId, NvPhysicalGpuHandle* hPhysicalGpu, NvU32* outputId) {
        *hPhysicalGpu = nullptr;
        *outputId = 0;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetObjectHandleForResource(IUnknown* invalid, IUnknown* pResource, NVDX_ObjectHandle* pHandle) {
        *pHandle = (NVDX_ObjectHandle)pResource;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetResourceHint() {
        return Error(NVAPI_NO_IMPLEMENTATION);
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetSleepStatus(IUnknown* pDevice, NV_GET_SLEEP_STATUS_PARAMS* pGetSleepStatusParams) {
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetLatency(IUnknown* pDev, NV_LATENCY_RESULT_PARAMS* pGetLatencyParams) {
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetSleepMode(IUnknown* pDevice, NV_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams) {
#if _MSC_VER
        if (pSetSleepModeParams->minimumIntervalUs > 0)
            antilag_ctx.set_fps_limit(1000000 / pSetSleepModeParams->minimumIntervalUs);
#endif
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetLatencyMarker(IUnknown* pDev, NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams) {
        if (!pDev)
            return Error();
        log(std::format("markerType: {}, frame id: {}", (unsigned int)pSetLatencyMarkerParams->markerType, (unsigned long long)pSetLatencyMarkerParams->frameID));
#if _MSC_VER
        antilag_ctx.init(pDev);
        switch (pSetLatencyMarkerParams->markerType) {
        case SIMULATION_START:
            if (antilag_ctx.calls_input_sample || antilag_ctx.calls_sleep) break;
            log(std::format("AntiLag update called on simulation start with result: {}", antilag_ctx.update()));
            break;
        case INPUT_SAMPLE:
            if (antilag_ctx.calls_sleep) break;
            antilag_ctx.calls_input_sample = true;
            log(std::format("AntiLag update called on input sample with result: {}", antilag_ctx.update()));
            break;
        }
#endif
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D_Sleep(IUnknown* pDevice) {
        if (!pDevice)
            return Error();
#if _MSC_VER
        antilag_ctx.init(pDevice);
        antilag_ctx.calls_sleep = true;
        log(std::format("AntiLag update called on sleep with result: {}", antilag_ctx.update()));
#endif
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown* invalid, NvU32 opCode, bool* pSupported) {
        *pSupported = true;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D11_BeginUAVOverlap(IUnknown* pDeviceOrContext) {
        static bool logged = false;
        if (!logged) {
            logged = true;
            return Ok();
        }
        else return NVAPI_OK; //return without logging
    }

    NvAPI_Status __cdecl NvAPI_D3D11_EndUAVOverlap(IUnknown* pDeviceOrContext) {
        static bool logged = false;
        if (!logged) {
            logged = true;
            return Ok();
        }
        else return NVAPI_OK; //return without logging
    }

    NvAPI_Status __cdecl NvAPI_D3D11_SetDepthBoundsTest(IUnknown* pDeviceOrContext) {
        static bool logged = false;
        if (!logged) {
            logged = true;
            return Ok();
        }
        else return NVAPI_OK; //return without logging
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetRaytracingCaps(IUnknown* invalid, NVAPI_D3D12_RAYTRACING_CAPS_TYPE type, void* pData, size_t dataSize) {
        if (pData == nullptr)
            return Error(NVAPI_INVALID_POINTER);

        switch (type) {
        case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_THREAD_REORDERING:
            if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAPS))
                return Error(NVAPI_INVALID_ARGUMENT);

            // let's hope that NvAPI_D3D12_IsNvShaderExtnOpCodeSupported returning false is enough to discourage games from attempting to use Shader Execution Reordering
            *(NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAPS*)pData = NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAP_NONE;
            break;

        case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_OPACITY_MICROMAP:
            if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAPS))
                return Error(NVAPI_INVALID_POINTER);

            *(NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAPS*)pData = NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAP_NONE;
            break;

        case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_DISPLACEMENT_MICROMAP:
            if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAPS))
                return Error(NVAPI_INVALID_POINTER);

            *(NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAPS*)pData = NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAP_NONE;
            break;

        default:
            return Error(NVAPI_INVALID_POINTER);
        }

        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(IUnknown* invalid, NvU32 opCode, bool* pSupported) {
        // VKD3D-Proton does not know any NVIDIA intrinsics
        *pSupported = false;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D12_SetNvShaderExtnSlotSpaceLocalThread() {
        return Ok();
    }

    static bool ConvertBuildRaytracingAccelerationStructureInputs(const NVAPI_D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS_EX* nvDesc, std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs, D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS* d3dDesc) {
        d3dDesc->Type = nvDesc->type;
        // assume that OMM via VK_EXT_opacity_micromap and DMM via VK_NV_displacement_micromap are not supported, allow only standard flags to be passed
        d3dDesc->Flags = static_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS>(nvDesc->flags & 0x3f);
        d3dDesc->NumDescs = nvDesc->numDescs;
        d3dDesc->DescsLayout = nvDesc->descsLayout;

        if (d3dDesc->Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL) {
            d3dDesc->InstanceDescs = nvDesc->instanceDescs;
            return true;
        }

        if (d3dDesc->Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL && d3dDesc->DescsLayout == D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS) {
            d3dDesc->ppGeometryDescs = reinterpret_cast<const D3D12_RAYTRACING_GEOMETRY_DESC* const*>(nvDesc->ppGeometryDescs);
            return true;
        }

        if (d3dDesc->Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL && d3dDesc->DescsLayout == D3D12_ELEMENTS_LAYOUT_ARRAY) {
            geometryDescs.resize(d3dDesc->NumDescs);

            for (unsigned i = 0; i < d3dDesc->NumDescs; ++i) {
                auto& d3dGeoDesc = geometryDescs[i];
                auto& nvGeoDesc = *reinterpret_cast<const NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX*>(reinterpret_cast<const std::byte*>(nvDesc->pGeometryDescs) + (i * nvDesc->geometryDescStrideInBytes));

                d3dGeoDesc.Flags = nvGeoDesc.flags;

                switch (nvGeoDesc.type) {
                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX:
                        d3dGeoDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
                        d3dGeoDesc.Triangles = nvGeoDesc.triangles;
                        break;
                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX:
                        d3dGeoDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
                        d3dGeoDesc.AABBs = nvGeoDesc.aabbs;
                        break;
                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_OMM_TRIANGLES_EX: // GetRaytracingCaps reports no OMM caps, we shouldn't reach this
                        log("Triangles with OMM attachment passed to acceleration structure build when OMM is not supported");
                        return false;
                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_DMM_TRIANGLES_EX: // GetRaytracingCaps reports no DMM caps, we shouldn't reach this
                        log("Triangles with DMM attachment passed to acceleration structure build when DMM is not supported");
                        return false;
                    default:
                        log("Unknown NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_EX");
                        return false;
                }
            }

            d3dDesc->pGeometryDescs = geometryDescs.data();
            return true;
        }

        return false;
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(ID3D12Device5* pDevice, NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS* pParams) {
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs{};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS desc{};

        if (!ConvertBuildRaytracingAccelerationStructureInputs(pParams->pDesc, geometryDescs, &desc))
            return Error(NVAPI_INVALID_ARGUMENT);

        pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&desc, pParams->pInfo);
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(ID3D12GraphicsCommandList4* pCommandList, const NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS* pParams) {
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs{};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {
            .DestAccelerationStructureData = pParams->pDesc->destAccelerationStructureData,
            .Inputs = {},
            .SourceAccelerationStructureData = pParams->pDesc->sourceAccelerationStructureData,
            .ScratchAccelerationStructureData = pParams->pDesc->scratchAccelerationStructureData,
        };

        if (!ConvertBuildRaytracingAccelerationStructureInputs(&pParams->pDesc->inputs, geometryDescs, &desc.Inputs))
            return Error(NVAPI_INVALID_ARGUMENT);

        pCommandList->BuildRaytracingAccelerationStructure(&desc, pParams->numPostbuildInfoDescs, pParams->pPostbuildInfoDescs);
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D12_NotifyOutOfBandCommandQueue(ID3D12CommandQueue* pCommandQueue, NV_OUT_OF_BAND_CQ_TYPE cqType) {
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D12_SetAsyncFrameMarker(ID3D12CommandQueue* pCommandQueue, NV_ASYNC_FRAME_MARKER_PARAMS* pSetAsyncFrameMarkerParams) {
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_DRS_CreateSession(NvDRSSessionHandle* session) {
        *session = drsSession;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_DRS_LoadSettings(NvDRSSessionHandle session) {
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetBaseProfile(NvDRSSessionHandle session, NvDRSProfileHandle* profile) {
        *profile = drsProfile;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId, NVDRS_SETTING* pSetting) {
        log(std::format("Missing setting: {}", settingId));
        return Error(NVAPI_SETTING_NOT_FOUND);
    }

    NvAPI_Status __cdecl NvAPI_DRS_DestroySession(NvDRSSessionHandle session) {
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_Unload() {
        return Ok();
    }

    NvAPI_Status __cdecl MISC_unknown(IUnknown* unknown, uint32_t* pMiscUnk) {
        std::fill(pMiscUnk, pMiscUnk + 4, 0x1);
        return Ok();
    }

    NvAPI_Status __cdecl MISC_vulkan(IUnknown* unknown) {
        return Ok();
    }
}