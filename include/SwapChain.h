#pragma once
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

using namespace std;

class SwapChain {

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> surfacePresentModes;

    
public:
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;

    void querySupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

private:
    VkSurfaceFormatKHR chooseSurfaceFormat();
    VkPresentModeKHR choosePresentMode();
    VkExtent2D chooseExtent();
};
