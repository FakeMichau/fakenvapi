#include "util.h"

namespace spoof {
    constexpr NvU32 driverVersion = 54629;
    constexpr std::string buildBranch = "buildBranch";
    constexpr NvU32 isDCHDriver = 1;
    constexpr NvU32 isNVIDIAStudioPackage = 1;
    constexpr NvU32 isNVIDIARTXProductionBranchPackage = 1;
    constexpr NvU32 isNVIDIARTXNewFeatureBranchPackage = 1;
    constexpr std::string buildBaseBranch = "buildBaseBranch";

    const std::string fullGPUName = "NVIDIA GeForce RTX 4090";
    constexpr NV_GPU_ARCHITECTURE_ID arch = NV_GPU_ARCHITECTURE_AD100;
    constexpr NV_GPU_ARCH_IMPLEMENTATION_ID implementation = NV_GPU_ARCH_IMPLEMENTATION_AD102;
    constexpr NV_GPU_CHIP_REVISION revision = NV_GPU_CHIP_REV_UNKNOWN;

    void driverInfo(NV_DISPLAY_DRIVER_INFO* driverInfo);
}