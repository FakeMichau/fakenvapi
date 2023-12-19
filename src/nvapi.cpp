#include "nvapi.h"

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
        return Error(NVAPI_NVIDIA_DISPLAY_NOT_FOUND);
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
        return Error(NVAPI_NO_IMPLEMENTATION);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetAllClockFrequencies(NvPhysicalGpuHandle hPhysicalGPU, NV_GPU_CLOCK_FREQUENCIES* pClkFreqs) {
        return Error(NVAPI_NOT_SUPPORTED);
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetDisplayIdByDisplayName(const char* displayName, NvU32* displayId) {
        *displayId = 0;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetGDIPrimaryDisplayId(NvU32* displayId) {
        *displayId = 0;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_Mosaic_GetDisplayViewportsByResolution(NvU32 displayId, NvU32 srcWidth, NvU32 srcHeight, NV_RECT viewports[NV_MOSAIC_MAX_DISPLAYS], NvU8* bezelCorrected) {
        return Error(NVAPI_MOSAIC_NOT_ACTIVE);
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDisplayDriverInfo(NV_DISPLAY_DRIVER_INFO* driverInfo) {
        spoof::driverInfo(driverInfo);
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
        *pDriverVersion = spoof::driverVersion;
        tonvss(szBuildBranchString, spoof::buildBranch);
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetObjectHandleForResource(IUnknown* invalid, IUnknown* pResource, NVDX_ObjectHandle* pHandle) {
        *pHandle = (NVDX_ObjectHandle)pResource;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetResourceHint() {
        return Error(NVAPI_NO_IMPLEMENTATION);
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
        *pSupported = true;
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D12_SetNvShaderExtnSlotSpaceLocalThread() {
        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(ID3D12Device5* pDevice, NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS* pParams) {
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs{};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS desc{};
        pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&desc, pParams->pInfo);
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

    NvAPI_Status __cdecl NvAPI_Unload() {
        return Ok();
    }
}