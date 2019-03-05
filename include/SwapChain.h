#pragma once
#include <iostream>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <QueueFamilyMap.h>

using namespace std;

class SwapChain {

	//support details, same for all swapchains
    static VkSurfaceCapabilitiesKHR availableSurfaceCapabilities;
    static std::vector<VkSurfaceFormatKHR> availableSurfaceFormats;
    static std::vector<VkPresentModeKHR> availableSurfacePresentModes;
    

public:

	static void querySupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	static bool hasAdequateSupport();

    VkSwapchainKHR vulkanHandle;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;

    void init(GLFWwindow* window, VkDevice device, VkSurfaceKHR surface, QueueFamilyMap queueFamilyMap, VkExtent2D appExtent);
    
	
	
private:

	
    VkSurfaceFormatKHR chooseSurfaceFormat() const;
    VkPresentModeKHR choosePresentMode() const;
    VkExtent2D chooseExtent(VkExtent2D appExtent) const;
	
};
