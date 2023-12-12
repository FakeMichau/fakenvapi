#include "util.h"

constexpr NvU32 driverVersion = 54629;
constexpr std::string buildBranch = "buildBranch";
constexpr NvU32 isDCHDriver = 1;
constexpr NvU32 isNVIDIAStudioPackage = 1;
constexpr NvU32 isNVIDIARTXProductionBranchPackage = 1;
constexpr NvU32 isNVIDIARTXNewFeatureBranchPackage = 1;
constexpr std::string buildBaseBranch = "buildBaseBranch";

const std::string fullGPUName = "NVIDIA GeForce RTX 4090";
// constexpr unsigned deviceId = 0x2684;
// constexpr unsigned vendorId = 0x10de;
// constexpr unsigned subSystemId = 0x88ac1043;

void spoofDriverInfo(NV_DISPLAY_DRIVER_INFO* driverInfo);