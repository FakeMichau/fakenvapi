#include "util.h"

constexpr NvU32 driverVersion = 54629;
constexpr std::string buildBranch = "buildBranch";
constexpr NvU32 isDCHDriver = 1;
constexpr NvU32 isNVIDIAStudioPackage = 1;
constexpr NvU32 isNVIDIARTXProductionBranchPackage = 1;
constexpr NvU32 isNVIDIARTXNewFeatureBranchPackage = 1;
constexpr std::string buildBaseBranch = "buildBaseBranch";

void spoofDriverInfo(NV_DISPLAY_DRIVER_INFO* driverInfo);