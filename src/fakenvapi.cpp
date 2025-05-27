#include "fakenvapi.h"

namespace nvd {
    bool Init() {
        if (!device_id) {
            IDXGIFactory1* factory = nullptr;
            if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory))) {
                spdlog::error("Failed to create DXGI Factory");
                return false;
            }

            IDXGIAdapter1* adapter = nullptr;
            if (FAILED(factory->EnumAdapters1(0, &adapter))) {
                spdlog::error("Failed to enumerate adapters");
                factory->Release();
                return false;
            }

            DXGI_ADAPTER_DESC1 adapter_desc;
            if (FAILED(adapter->GetDesc1(&adapter_desc))) {
                spdlog::error("Failed to get adapter description");
                adapter->Release();
                factory->Release();
                return false;
            }

            luid = adapter_desc.AdapterLuid;
            device_id = adapter_desc.DeviceId;
            vendor_id = adapter_desc.VendorId;
            subsystem_id = adapter_desc.SubSysId;
            revision_id = adapter_desc.Revision;

            adapter->Release();
            factory->Release();

            lowlatency_ctx.init_lfx();
        }

        return true;
    }

    NvAPI_Status __cdecl NvAPI_Initialize() {
        ref_count++;
        
        if (!Init())
            return ERROR();

        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GetInterfaceVersionString(NvAPI_ShortString desc) {
        tonvss(desc, "fakenvapi");
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS], NvU32* count) {
        handles[0] = nullptr;
        *count = 1;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_EnumLogicalGPUs(NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS], NvU32* count) {
        handles[0] = nullptr;
        *count = 1;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_EnumNvidiaDisplayHandle(NvU32 displayId, NvDisplayHandle* handle) {
        if (displayId == 0) {
            return OK();
        }
        return ERROR_VALUE(NVAPI_END_ENUMERATION);
    }

    NvAPI_Status __cdecl NvAPI_GetLogicalGPUFromPhysicalGPU(NvPhysicalGpuHandle physicalHandle, NvLogicalGpuHandle* logicalHandle) {
        *logicalHandle = nullptr;
        return OK();
    }


    NvAPI_Status __cdecl NvAPI_GetGPUIDfromPhysicalGPU(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pGpuId) {
        *pGpuId = 42;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUFromGPUID(NvU32 gpuId, NvPhysicalGpuHandle* pPhysicalGPU) {
        *pPhysicalGPU = nullptr;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromDisplay(NvDisplayHandle hNvDisp, NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32* pGpuCount) {
        nvGPUHandle[0] = nullptr;
        *pGpuCount = 1;

        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromLogicalGPU(NvLogicalGpuHandle hLogicalGPU, NvPhysicalGpuHandle hPhysicalGPU[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount) {
        hPhysicalGPU[0] = nullptr;
        *pGpuCount = 1;

        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GetErrorMessage(NvAPI_Status status, NvAPI_ShortString szMsg) {
        std::string error = from_error_nr(status);
        spdlog::error("NvAPI_GetErrorMessage gave this error: {}", error);
        tonvss(szMsg, error);
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GetDisplayDriverVersion(NvDisplayHandle hNvDisplay, NV_DISPLAY_DRIVER_VERSION *pVersion) {
        if (!pVersion)
            return ERROR_VALUE(NVAPI_INVALID_ARGUMENT);

        pVersion->drvVersion = 99999;
        tonvss(pVersion->szBuildBranchString, "buildBranch");
        tonvss(pVersion->szAdapterString, "NVIDIA GeForce RTX 4090");
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GPU_CudaEnumComputeCapableGpus(NV_COMPUTE_GPU_TOPOLOGY* pComputeTopo) {
        auto compute_topoV1 = reinterpret_cast<NV_COMPUTE_GPU_TOPOLOGY_V1*>(pComputeTopo);
        compute_topoV1->gpuCount = 1;
        compute_topoV1->computeGpus[0].hPhysicalGpu = nullptr;
        compute_topoV1->computeGpus[0].flags = NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_CAPABLE | NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_ENABLE | NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_RECOMMENDED;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetConnectedDisplayIds(NvPhysicalGpuHandle handle, NV_GPU_DISPLAYIDS* displayIds, NvU32* displayCount, NvU32 flags) {
        *displayCount = 0;  // no displays connected, may cause issues
        // NVAPI_NVIDIA_DISPLAY_NOT_FOUND could be considered
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetArchInfo(NvPhysicalGpuHandle handle, NV_GPU_ARCH_INFO* archInfo) {
        archInfo->architecture = NV_GPU_ARCHITECTURE_AD100;
        archInfo->architecture_id = NV_GPU_ARCHITECTURE_AD100;
        archInfo->implementation = NV_GPU_ARCH_IMPLEMENTATION_AD102;
        archInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_AD102;
        archInfo->revision = NV_GPU_CHIP_REV_UNKNOWN;
        archInfo->revision_id = NV_GPU_CHIP_REV_UNKNOWN;

        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetLogicalGpuInfo(NvLogicalGpuHandle logicalHandle, NV_LOGICAL_GPU_DATA* logicalGpuData) {
        if (!Init())
            return ERROR();

        memcpy(logicalGpuData->pOSAdapterId, &luid, sizeof(luid));
        logicalGpuData->physicalGpuHandles[0] = nullptr;
        logicalGpuData->physicalGpuCount = 1;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pDeviceId, NvU32* pSubSystemId, NvU32* pRevisionId, NvU32* pExtDeviceId) {
        if (!Init())
            return ERROR();

        *pDeviceId = (device_id << 16) | vendor_id;
        *pSubSystemId = subsystem_id;
        *pRevisionId = revision_id;
        *pExtDeviceId = device_id;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName) {
        tonvss(szName, "NVIDIA GeForce RTX 4090");
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetGpuCoreCount(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pCount) {
        *pCount = 1;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetAllClockFrequencies(NvPhysicalGpuHandle hPhysicalGPU, NV_GPU_CLOCK_FREQUENCIES* pClkFreqs) {
        if (pClkFreqs == nullptr)
            return ERROR_VALUE(NVAPI_INVALID_ARGUMENT);

        if (pClkFreqs->version != NV_GPU_CLOCK_FREQUENCIES_VER_1 && pClkFreqs->version != NV_GPU_CLOCK_FREQUENCIES_VER_2 && pClkFreqs->version != NV_GPU_CLOCK_FREQUENCIES_VER_3)
            return ERROR_VALUE(NVAPI_INCOMPATIBLE_STRUCT_VERSION);

        if (pClkFreqs->ClockType != static_cast<unsigned int>(NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ)) {
            return ERROR_VALUE(NVAPI_NOT_SUPPORTED);
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
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetAdapterIdFromPhysicalGpu(NvPhysicalGpuHandle hPhysicalGpu, void* pOSAdapterId) {
        if (!Init())
            return ERROR();

        memcpy(pOSAdapterId, &luid, sizeof(luid));
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPstates20(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_PERF_PSTATES20_INFO* pPstatesInfo) {
        if (!pPstatesInfo)
            return ERROR();

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
        memcpy(&pPstatesInfo->pstates[1], &pPstatesInfo->pstates[0], sizeof(pPstatesInfo->pstates[0]));
        pPstatesInfo->pstates[1].pstateId = NVAPI_GPU_PERF_PSTATE_P1;

        return OK();
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetDisplayIdByDisplayName(const char* displayName, NvU32* displayId) {
        *displayId = 0;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetGDIPrimaryDisplayId(NvU32* displayId) {
        *displayId = 0;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_Disp_SetOutputMode(NvU32 displayId, NV_DISPLAY_OUTPUT_MODE* pDisplayMode) {
        *pDisplayMode = NV_DISPLAY_OUTPUT_MODE_SDR; // meaning no HDR
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_Disp_GetOutputMode(NvU32 displayId, NV_DISPLAY_OUTPUT_MODE* pDisplayMode) {
        *pDisplayMode = NV_DISPLAY_OUTPUT_MODE_SDR; // meaning no HDR
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_Disp_GetHdrCapabilities(NvU32 displayId, NV_HDR_CAPABILITIES *pHdrCapabilities) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_Disp_HdrColorControl(NvU32 displayId, NV_HDR_COLOR_DATA *pHdrColorData) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_Mosaic_GetDisplayViewportsByResolution(NvU32 displayId, NvU32 srcWidth, NvU32 srcHeight, NV_RECT viewports[NV_MOSAIC_MAX_DISPLAYS], NvU8* bezelCorrected) {
        for (int i = 0; i < NV_MOSAIC_MAX_DISPLAYS; i++) {
            viewports[i].top = 0;
            viewports[i].left = 0;
            viewports[i].right = 0;
            viewports[i].bottom = 0;
        }
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDisplayDriverInfo(NV_DISPLAY_DRIVER_INFO* driverInfo) {
        driverInfo->driverVersion = 99999;
        tonvss(driverInfo->szBuildBranch, "buildBranch");
        driverInfo->bIsDCHDriver = 1;
        driverInfo->bIsNVIDIAStudioPackage = 1;
        driverInfo->bIsNVIDIARTXProductionBranchPackage = 1;
        driverInfo->bIsNVIDIARTXNewFeatureBranchPackage = 1;
        if (driverInfo->version == 2)
            tonvss(driverInfo->szBuildBaseBranch, "buildBaseBranch");
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
        *pDriverVersion = 99999;
        tonvss(szBuildBranchString, "buildBranch");
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDisplayIdFromGpuAndOutputId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputId, NvU32* displayId) {
        *displayId = 0;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetGpuAndOutputIdFromDisplayId(NvU32 displayId, NvPhysicalGpuHandle* hPhysicalGpu, NvU32* outputId) {
        *hPhysicalGpu = nullptr;
        *outputId = 0;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetObjectHandleForResource(IUnknown* invalid, IUnknown* pResource, NVDX_ObjectHandle* pHandle) {
        *pHandle = (NVDX_ObjectHandle)pResource;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetResourceHint() {
        return ERROR_VALUE(NVAPI_NO_IMPLEMENTATION);
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetSleepStatus(IUnknown* pDevice, NV_GET_SLEEP_STATUS_PARAMS* pGetSleepStatusParams) {
        pGetSleepStatusParams->bLowLatencyMode = lowlatency_ctx.active;
        pGetSleepStatusParams->bFsVrr = true;
        pGetSleepStatusParams->bCplVsyncOn = true;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetLatency(IUnknown* pDev, NV_LATENCY_RESULT_PARAMS* pGetLatencyParams) {
        if (!Init())
            return ERROR();

        if (pGetLatencyParams == nullptr)
            return ERROR_VALUE(NVAPI_INVALID_ARGUMENT);
        
        // xellGetFramesReports() currently doesn't give any extra data that we can't already get
        memcpy(&pGetLatencyParams->frameReport, &lowlatency_ctx.frame_reports, sizeof(lowlatency_ctx.frame_reports));

        // FrameReport* frame_reports = reinterpret_cast<FrameReport*>(pGetLatencyParams->frameReport);
        // std::sort(frame_reports, frame_reports + 64, [](const FrameReport &a, const FrameReport &b) {
        //     return a.frameID < b.frameID;
        // });

        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetSleepMode(IUnknown* pDevice, NV_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams) {
        if (!Init())
            return ERROR();

        static bool previous_boost = false;
        if (lowlatency_ctx.active != pSetSleepModeParams->bLowLatencyMode || previous_boost != pSetSleepModeParams->bLowLatencyBoost) {
            spdlog::info(
                "Changed reflex settings to: {}, boost: {}", 
                pSetSleepModeParams->bLowLatencyMode ? "enabled" : "disabled", 
                pSetSleepModeParams->bLowLatencyBoost ? "enabled" : "disabled"
            );
            lowlatency_ctx.active = pSetSleepModeParams->bLowLatencyMode;
            previous_boost = pSetSleepModeParams->bLowLatencyBoost;
        }
        lowlatency_ctx.set_min_interval_us(pSetSleepModeParams->minimumIntervalUs);

        lowlatency_ctx.xell_set_sleep(pSetSleepModeParams);

        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetLatencyMarker(IUnknown* pDev, NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams) {
        if (!Init())
            return ERROR();

        if (!pDev)
            return ERROR();

        lowlatency_ctx.init_al2(pDev);
        lowlatency_ctx.init_xell(pDev);

        lowlatency_ctx.handle_marker(pSetLatencyMarkerParams);

        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D_Sleep(IUnknown* pDevice) {
        if (!Init())
            return ERROR();
            
        if (!pDevice)
            return ERROR();

        if (lowlatency_ctx.get_mode() == Mode::LatencyFlex && lowlatency_ctx.lfx_mode == LFXMode::ReflexIDs)
            return OK();

        lowlatency_ctx.init_al2(pDevice);
        lowlatency_ctx.init_xell(pDevice);

        lowlatency_ctx.sleep_called();
        spdlog::debug("LowLatency update called on sleep with result: {}", lowlatency_ctx.update(0));
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetReflexSync(IUnknown* pDev, NV_SET_REFLEX_SYNC_PARAMS* pSetReflexSyncParams) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown* invalid, NvU32 opCode, bool* pSupported) {
        *pSupported = true;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D11_BeginUAVOverlap(IUnknown* pDeviceOrContext) {
        static bool logged = false;
        if (!logged) {
            logged = true;
            return OK();
        }
        else return NVAPI_OK; //return without logging
    }

    NvAPI_Status __cdecl NvAPI_D3D11_EndUAVOverlap(IUnknown* pDeviceOrContext) {
        static bool logged = false;
        if (!logged) {
            logged = true;
            return OK();
        }
        else return NVAPI_OK; //return without logging
    }

    NvAPI_Status __cdecl NvAPI_D3D11_SetDepthBoundsTest(IUnknown* pDeviceOrContext) {
        static bool logged = false;
        if (!logged) {
            logged = true;
            return OK();
        }
        else return NVAPI_OK; //return without logging
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetRaytracingCaps(IUnknown* invalid, NVAPI_D3D12_RAYTRACING_CAPS_TYPE type, void* pData, size_t dataSize) {
        if (pData == nullptr)
            return ERROR_VALUE(NVAPI_INVALID_POINTER);

        switch (type) {
        case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_THREAD_REORDERING:
            if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAPS))
                return ERROR_VALUE(NVAPI_INVALID_ARGUMENT);

            // let's hope that NvAPI_D3D12_IsNvShaderExtnOpCodeSupported returning false is enough to discourage games from attempting to use Shader Execution Reordering
            *(NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAPS*)pData = NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAP_NONE;
            break;

        case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_OPACITY_MICROMAP:
            if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAPS))
                return ERROR_VALUE(NVAPI_INVALID_POINTER);

            *(NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAPS*)pData = NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAP_NONE;
            break;

        case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_DISPLACEMENT_MICROMAP:
            if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAPS))
                return ERROR_VALUE(NVAPI_INVALID_POINTER);

            *(NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAPS*)pData = NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAP_NONE;
            break;

        default:
            return ERROR_VALUE(NVAPI_INVALID_POINTER);
        }

        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(IUnknown* invalid, NvU32 opCode, bool* pSupported) {
        // VKD3D-Proton does not know any NVIDIA intrinsics
        *pSupported = false;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D12_SetNvShaderExtnSlotSpaceLocalThread() {
        return OK();
    }

    // Taken directly from dxvk-nvapi
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
                        spdlog::error("Triangles with OMM attachment passed to acceleration structure build when OMM is not supported");
                        return false;
                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_DMM_TRIANGLES_EX: // GetRaytracingCaps reports no DMM caps, we shouldn't reach this
                        spdlog::error("Triangles with DMM attachment passed to acceleration structure build when DMM is not supported");
                        return false;
                    default:
                        spdlog::error("Unknown NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_EX");
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
            return ERROR_VALUE(NVAPI_INVALID_ARGUMENT);

        pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&desc, pParams->pInfo);
        return OK();
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
            return ERROR_VALUE(NVAPI_INVALID_ARGUMENT);

        pCommandList->BuildRaytracingAccelerationStructure(&desc, pParams->numPostbuildInfoDescs, pParams->pPostbuildInfoDescs);
        static bool logged = false;
        if (!logged) {
            logged = true;
            return OK();
        }
        else return NVAPI_OK; //return without logging
    }

    NvAPI_Status __cdecl NvAPI_D3D12_NotifyOutOfBandCommandQueue(ID3D12CommandQueue* pCommandQueue, NV_OUT_OF_BAND_CQ_TYPE cqType) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_D3D12_SetAsyncFrameMarker(ID3D12CommandQueue* pCommandQueue, NV_ASYNC_FRAME_MARKER_PARAMS* pSetAsyncFrameMarkerParams) {
        static NvU64 previous_frame_id = 0;
        static NvU64 current_frame_id = 0;
        switch (pSetAsyncFrameMarkerParams->markerType) {
            case PRESENT_START:
                log_event("async_marker_PRESENT_START", "{}", pSetAsyncFrameMarkerParams->frameID);
                break;
            case PRESENT_END:
                log_event("async_marker_PRESENT_END", "{}", pSetAsyncFrameMarkerParams->frameID);
                break;
            case OUT_OF_BAND_RENDERSUBMIT_START:
                log_event("async_marker_OUB_RENDERSUBMIT_START", "{}", pSetAsyncFrameMarkerParams->frameID);
                break;
            case OUT_OF_BAND_RENDERSUBMIT_END:
                log_event("async_marker_OUB_RENDERSUBMIT_END", "{}", pSetAsyncFrameMarkerParams->frameID);
                break;
            case OUT_OF_BAND_PRESENT_START: {
                log_event("async_marker_OUB_PRESENT_START", "{}", pSetAsyncFrameMarkerParams->frameID);
                constexpr size_t history_size = 12;
                static size_t counter = 0;
                static NvU64 previous_frame_ids[history_size] = {};
                current_frame_id = pSetAsyncFrameMarkerParams->frameID;

                previous_frame_ids[counter%history_size] = current_frame_id;
                counter++;

                int repeat_count = 0;

                for (size_t i = 1; i < history_size; i++) {
                    // won't catch repeat frame ids across array wrap around
                    if (previous_frame_ids[i] == previous_frame_ids[i - 1]) {
                        repeat_count++;
                    }
                }

                if (lowlatency_ctx.fg && repeat_count == 0) lowlatency_ctx.fg = false;
                else if (!lowlatency_ctx.fg && repeat_count >= history_size / 2 - 1) lowlatency_ctx.fg = true;

                lowlatency_ctx.set_fg_type(previous_frame_id != current_frame_id, current_frame_id);
                previous_frame_id = current_frame_id;
                break;
            }
            case OUT_OF_BAND_PRESENT_END:
                log_event("async_marker_OUB_PRESENT_END", "{}", pSetAsyncFrameMarkerParams->frameID);
                if (previous_frame_id == current_frame_id)
                    lowlatency_ctx.pcl_end(pSetAsyncFrameMarkerParams->frameID);
                break;
            default:
                log_event("async_marker_other", "{}", pSetAsyncFrameMarkerParams->frameID);
                break;
        }
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_DRS_CreateSession(NvDRSSessionHandle* session) {
        *session = drs_session;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_DRS_LoadSettings(NvDRSSessionHandle session) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_DRS_SaveSettings(NvDRSSessionHandle session) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetBaseProfile(NvDRSSessionHandle session, NvDRSProfileHandle* profile) {
        *profile = drs_profile;
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId, NVDRS_SETTING* pSetting) {
        spdlog::debug("Missing get setting: {}", settingId);
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_DRS_SetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NVDRS_SETTING *pSetting) {
        spdlog::debug("Missing set setting: {}", pSetting->settingId);
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_DRS_DestroySession(NvDRSSessionHandle session) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_Unknown_1(IUnknown* unknown, uint32_t* pMiscUnk) {
        std::fill(pMiscUnk, pMiscUnk + 4, 0x1);
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_Vulkan_1(IUnknown* unknown) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_SK_1(IUnknown* unknown) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_SK_2(IUnknown* unknown) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_SK_3(IUnknown* unknown) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_SK_4(IUnknown* unknown) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_SK_5(IUnknown* unknown) {
        return OK();
    }

    NvAPI_Status __cdecl NvAPI_Unload() {
        if(ref_count.load() > 0)
            ref_count--;
        
        if(ref_count.load() == 0)
            lowlatency_ctx.unload();

        return OK();
    }

    NvAPI_Status __cdecl Fake_GetLatency(uint64_t* call_spot, uint64_t* target, uint64_t* latency, uint64_t* frame_time) {
        if (!call_spot || !target || !latency || !frame_time) return ERROR_VALUE(NVAPI_INVALID_POINTER);

        if (lowlatency_ctx.get_mode() != Mode::LatencyFlex) return ERROR_VALUE(NVAPI_DATA_NOT_FOUND);
        *call_spot = (uint64_t)lowlatency_ctx.call_spot;

        *target = lowlatency_ctx.lfx_stats.target;
        *latency = lowlatency_ctx.lfx_stats.latency;
        *frame_time = lowlatency_ctx.lfx_stats.frame_time;

        return OK();
    }

    NvAPI_Status __cdecl Fake_InformFGState(bool fg_state) {
        lowlatency_ctx.forced_fg = fg_state;
        return OK();
    }

    NvAPI_Status __cdecl Fake_InformPresentFG(bool frame_interpolated, uint64_t reflex_frame_id) {
        lowlatency_ctx.set_fg_type(frame_interpolated, reflex_frame_id);
        return OK();
    }

#if _WIN64
    NvAPI_Status __cdecl Fake_GetAntiLagCtx(AMD::AntiLag2DX12::Context** al2_context) {
        if (al2_context == nullptr)
            return ERROR_VALUE(NVAPI_INVALID_ARGUMENT);

        if (lowlatency_ctx.al2_dx12_ctx.m_pAntiLagAPI != nullptr) {
            *al2_context = &lowlatency_ctx.al2_dx12_ctx;
            return OK();
        }

        return ERROR();
    }
#endif
}