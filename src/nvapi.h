#include <algorithm>
#include <unordered_map>
#include <format>

#include <dxgi.h>
#include "../include/d3d12.h"
#include <vector>

#include "util.h"
#include "spoofInfo.h"
#include "log.h"

static auto drs = 1U;
static auto drsSession = reinterpret_cast<NvDRSSessionHandle>(&drs);
static auto drsProfile = reinterpret_cast<NvDRSProfileHandle>(&drs);


static LUID luid;
static UINT deviceId;
static UINT vendorId;
static UINT subSysId;
static UINT revisionId;

class Adapter {};
Adapter* dummy{};