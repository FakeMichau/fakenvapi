#pragma once

#include "log.h"

#include <detours.h>
#include <vulkan/vulkan_core.h>
#include <vector>

class VulkanHooks {
private:
    static PFN_vkCreateDevice o_vkCreateDevice;
    static PFN_vkGetPhysicalDeviceFeatures2 o_vkGetPhysicalDeviceFeatures2;
    static PFN_vkGetDeviceProcAddr o_vkGetDeviceProcAddr;

public:
    static PFN_vkAntiLagUpdateAMD o_vkAntiLagUpdateAMD;
    static void hook_vulkan(HMODULE vulkanModule);
    static VkResult hkvkCreateDevice(VkPhysicalDevice physicalDevice, VkDeviceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDevice *pDevice);
};
