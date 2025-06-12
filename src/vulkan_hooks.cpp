#include "vulkan_hooks.h"

#include <detours.h>
#include <vulkan/vulkan_core.h>
#include <vector>

PFN_vkCreateDevice VulkanHooks::o_vkCreateDevice = nullptr;
PFN_vkGetPhysicalDeviceFeatures2 VulkanHooks::o_vkGetPhysicalDeviceFeatures2 = nullptr;
PFN_vkGetDeviceProcAddr VulkanHooks::o_vkGetDeviceProcAddr = nullptr;
PFN_vkAntiLagUpdateAMD VulkanHooks::o_vkAntiLagUpdateAMD = nullptr;

VkResult VulkanHooks::hkvkCreateDevice(VkPhysicalDevice physicalDevice, VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    spdlog::debug("hkvkCreateDevice");

    std::vector<const char*> extensions(pCreateInfo->ppEnabledExtensionNames, pCreateInfo->ppEnabledExtensionNames + pCreateInfo->enabledExtensionCount);

    // AntiLag
    bool antiLagSupported = false;
    VkPhysicalDeviceFeatures2 features2 = {};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    VkPhysicalDeviceAntiLagFeaturesAMD antiLagFeatures = {};
    antiLagFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD;

    features2.pNext = &antiLagFeatures;

    if (o_vkGetPhysicalDeviceFeatures2) {
        o_vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

        if (antiLagFeatures.antiLag) {
            antiLagSupported = true;
            spdlog::info("Adding AntiLag extension");
            extensions.push_back(VK_AMD_ANTI_LAG_EXTENSION_NAME);
        }
    }

    // TODO add check for VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME as AntiLag doesn't support it
    
    pCreateInfo->enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    pCreateInfo->ppEnabledExtensionNames = extensions.data();

    auto result = o_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

    // Can ask for a function from an extension after the device creation
    if (antiLagSupported && o_vkGetDeviceProcAddr) {
        o_vkAntiLagUpdateAMD = (PFN_vkAntiLagUpdateAMD) o_vkGetDeviceProcAddr(*pDevice, "vkAntiLagUpdateAMD");
    } else {
        spdlog::info("Vulkan AntiLag can't be enabled");
    }

    return result;
}
void VulkanHooks::hook_vulkan(HMODULE vulkanModule) {
    spdlog::debug("Trying to hook Vulkan");

    o_vkCreateDevice = (PFN_vkCreateDevice) GetProcAddress(vulkanModule, "vkCreateDevice");
    o_vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2) GetProcAddress(vulkanModule, "vkGetPhysicalDeviceFeatures2");
    o_vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) GetProcAddress(vulkanModule, "vkGetDeviceProcAddr");

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_vkCreateDevice)
        DetourAttach(&(PVOID&) o_vkCreateDevice, hkvkCreateDevice);

    DetourTransactionCommit();
}