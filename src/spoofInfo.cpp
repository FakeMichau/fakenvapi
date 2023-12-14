#include "spoofInfo.h"

namespace spoof {
    void driverInfo(NV_DISPLAY_DRIVER_INFO* driverInfo) {
        driverInfo->driverVersion = driverVersion;
        tonvss(driverInfo->szBuildBranch, buildBranch);
        driverInfo->bIsDCHDriver = isDCHDriver;
        driverInfo->bIsNVIDIAStudioPackage = isNVIDIAStudioPackage;
        driverInfo->bIsNVIDIARTXProductionBranchPackage = isNVIDIARTXProductionBranchPackage;
        driverInfo->bIsNVIDIARTXNewFeatureBranchPackage = isNVIDIARTXNewFeatureBranchPackage;
        if (driverInfo->version == 2)
            tonvss(driverInfo->szBuildBaseBranch, buildBaseBranch);
    }
}