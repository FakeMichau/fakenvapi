#pragma once
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <numeric>
#include <format>

#include <dxgi.h>
#if defined __MINGW64__ || defined __MINGW32__
#include "../include/d3d12.h"
#else
#include <d3d12.h>
#endif
#include <vector>

#include "lowlatency.h"

#include "util.h"
#include "spoofInfo.h"
#include "log.h"

namespace nvd {
    static auto drs = 1U;
    static auto drsSession = reinterpret_cast<NvDRSSessionHandle>(&drs);
    static auto drsProfile = reinterpret_cast<NvDRSProfileHandle>(&drs);

    static LUID luid;
    static UINT deviceId;
    static UINT vendorId;
    static UINT subSysId;
    static UINT revisionId;

    static LowLatency lowlatency_ctx;

    NvAPI_Status __cdecl NvAPI_Initialize();
    NvAPI_Status __cdecl NvAPI_GetInterfaceVersionString(NvAPI_ShortString desc);
    NvAPI_Status __cdecl NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS], NvU32* count);
    NvAPI_Status __cdecl NvAPI_EnumLogicalGPUs(NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS], NvU32* count);
    NvAPI_Status __cdecl NvAPI_EnumNvidiaDisplayHandle(NvU32 displayId, NvDisplayHandle* handle);
    NvAPI_Status __cdecl NvAPI_GetLogicalGPUFromPhysicalGPU(NvPhysicalGpuHandle physicalHandle, NvLogicalGpuHandle* logicalHandle);
    NvAPI_Status __cdecl NvAPI_GetGPUIDfromPhysicalGPU(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pGpuId);
    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUFromGPUID(NvU32 gpuId, NvPhysicalGpuHandle* pPhysicalGPU);
    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromDisplay(NvDisplayHandle hNvDisp, NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount);
    NvAPI_Status __cdecl NvAPI_GetErrorMessage(NvAPI_Status status, NvAPI_ShortString szMsg);
    NvAPI_Status __cdecl NvAPI_GPU_CudaEnumComputeCapableGpus(NV_COMPUTE_GPU_TOPOLOGY* pComputeTopo);
    NvAPI_Status __cdecl NvAPI_GPU_GetConnectedDisplayIds(NvPhysicalGpuHandle handle, NV_GPU_DISPLAYIDS* displayIds, NvU32* displayCount, NvU32 flags);
    NvAPI_Status __cdecl NvAPI_GPU_GetArchInfo(NvPhysicalGpuHandle handle, NV_GPU_ARCH_INFO* archInfo);
    NvAPI_Status __cdecl NvAPI_GPU_GetLogicalGpuInfo(NvLogicalGpuHandle logicalHandle, NV_LOGICAL_GPU_DATA* logicalGpuData);
    NvAPI_Status __cdecl NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pDeviceId, NvU32* pSubSystemId, NvU32* pRevisionId, NvU32* pExtDeviceId);
    NvAPI_Status __cdecl NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName);
    NvAPI_Status __cdecl NvAPI_GPU_GetGpuCoreCount(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pCount);
    NvAPI_Status __cdecl NvAPI_GPU_GetAllClockFrequencies(NvPhysicalGpuHandle hPhysicalGPU, NV_GPU_CLOCK_FREQUENCIES* pClkFreqs);
    NvAPI_Status __cdecl NvAPI_GPU_GetAdapterIdFromPhysicalGpu(NvPhysicalGpuHandle hPhysicalGpu, void* pOSAdapterId);
    NvAPI_Status __cdecl NvAPI_GPU_GetPstates20(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_PERF_PSTATES20_INFO* pPstatesInfo);
    NvAPI_Status __cdecl NvAPI_DISP_GetDisplayIdByDisplayName(const char* displayName, NvU32* displayId);
    NvAPI_Status __cdecl NvAPI_DISP_GetGDIPrimaryDisplayId(NvU32* displayId);
    NvAPI_Status __cdecl NvAPI_Disp_SetOutputMode(NvU32 displayId, NV_DISPLAY_OUTPUT_MODE* pDisplayMode);
    NvAPI_Status __cdecl NvAPI_Disp_GetOutputMode(NvU32 displayId, NV_DISPLAY_OUTPUT_MODE* pDisplayMode);
    NvAPI_Status __cdecl NvAPI_Mosaic_GetDisplayViewportsByResolution(NvU32 displayId, NvU32 srcWidth, NvU32 srcHeight, NV_RECT viewports[NV_MOSAIC_MAX_DISPLAYS], NvU8* bezelCorrected);
    NvAPI_Status __cdecl NvAPI_SYS_GetDisplayDriverInfo(NV_DISPLAY_DRIVER_INFO* driverInfo);
    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString);
    NvAPI_Status __cdecl NvAPI_SYS_GetDisplayIdFromGpuAndOutputId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 outputId, NvU32* displayId);
    NvAPI_Status __cdecl NvAPI_SYS_GetGpuAndOutputIdFromDisplayId(NvU32 displayId, NvPhysicalGpuHandle* hPhysicalGpu, NvU32* outputId);
    NvAPI_Status __cdecl NvAPI_D3D_GetObjectHandleForResource(IUnknown* invalid, IUnknown* pResource, NVDX_ObjectHandle* pHandle);
    NvAPI_Status __cdecl NvAPI_D3D_SetResourceHint();
    NvAPI_Status __cdecl NvAPI_D3D_GetSleepStatus(IUnknown* pDevice, NV_GET_SLEEP_STATUS_PARAMS* pGetSleepStatusParams);
    NvAPI_Status __cdecl NvAPI_D3D_GetLatency(IUnknown* pDev, NV_LATENCY_RESULT_PARAMS* pGetLatencyParams);
    NvAPI_Status __cdecl NvAPI_D3D_SetSleepMode(IUnknown* pDevice, NV_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams);
    NvAPI_Status __cdecl NvAPI_D3D_SetLatencyMarker(IUnknown* pDev, NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams);
    NvAPI_Status __cdecl NvAPI_D3D_Sleep(IUnknown* pDevice);
    NvAPI_Status __cdecl NvAPI_D3D_SetReflexSync(IUnknown* pDev, NV_SET_REFLEX_SYNC_PARAMS* pSetReflexSyncParams);
    NvAPI_Status __cdecl NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown* invalid, NvU32 opCode, bool* pSupported);
    NvAPI_Status __cdecl NvAPI_D3D11_BeginUAVOverlap(IUnknown* pDeviceOrContext);
    NvAPI_Status __cdecl NvAPI_D3D11_EndUAVOverlap(IUnknown* pDeviceOrContext);
    NvAPI_Status __cdecl NvAPI_D3D11_SetDepthBoundsTest(IUnknown* pDeviceOrContext);
    NvAPI_Status __cdecl NvAPI_D3D12_GetRaytracingCaps(IUnknown* invalid, NVAPI_D3D12_RAYTRACING_CAPS_TYPE type, void* pData, size_t dataSize);
    NvAPI_Status __cdecl NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(IUnknown* invalid, NvU32 opCode, bool* pSupported);
    NvAPI_Status __cdecl NvAPI_D3D12_SetNvShaderExtnSlotSpaceLocalThread();
    NvAPI_Status __cdecl NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(ID3D12Device5* pDevice, NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS* pParams);
    NvAPI_Status __cdecl NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(ID3D12GraphicsCommandList4* pCommandList, const NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS* pParams);
    NvAPI_Status __cdecl NvAPI_D3D12_NotifyOutOfBandCommandQueue(ID3D12CommandQueue* pCommandQueue, NV_OUT_OF_BAND_CQ_TYPE cqType);
    NvAPI_Status __cdecl NvAPI_D3D12_SetAsyncFrameMarker(ID3D12CommandQueue* pCommandQueue, NV_ASYNC_FRAME_MARKER_PARAMS* pSetAsyncFrameMarkerParams);
    NvAPI_Status __cdecl NvAPI_DRS_CreateSession(NvDRSSessionHandle* session);
    NvAPI_Status __cdecl NvAPI_DRS_LoadSettings(NvDRSSessionHandle session);
    NvAPI_Status __cdecl NvAPI_DRS_SaveSettings(NvDRSSessionHandle session);
    NvAPI_Status __cdecl NvAPI_DRS_GetBaseProfile(NvDRSSessionHandle session, NvDRSProfileHandle* profile);
    NvAPI_Status __cdecl NvAPI_DRS_GetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId, NVDRS_SETTING* pSetting);
    NvAPI_Status __cdecl NvAPI_DRS_SetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NVDRS_SETTING *pSetting);
    NvAPI_Status __cdecl NvAPI_DRS_DestroySession(NvDRSSessionHandle session);
    NvAPI_Status __cdecl NvAPI_Unknown_1(IUnknown* unknown, uint32_t* pMiscUnk);
    NvAPI_Status __cdecl NvAPI_Vulkan_1(IUnknown* unknown);
    NvAPI_Status __cdecl NvAPI_Unload();
    NvAPI_Status __cdecl Dummy_GetLatency(uint64_t* call_spot, uint64_t* waitTarget, uint64_t* latency, uint64_t* frameTime);
}