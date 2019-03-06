#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <QueueFamilyMap.h>
#include <Utils.h>
using namespace std;

class SwapChain {

	//support details, same for all swapchains
    static VkSurfaceCapabilitiesKHR availableSurfaceCapabilities;
    static std::vector<VkSurfaceFormatKHR> availableSurfaceFormats;
    static std::vector<VkPresentModeKHR> availableSurfacePresentModes;
    

public:

	//choices, also same for all swapchains
	static VkSurfaceFormatKHR surfaceFormat;
	static VkPresentModeKHR presentMode;
	static VkExtent2D extent;

	static void querySupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	static bool hasAdequateSupport();


    static VkSwapchainKHR vulkanHandle;
    static std::vector<VkImage> images;
    static std::vector<VkImageView> imageViews;

    

    static void init(GLFWwindow* window, VkDevice device, VkSurfaceKHR surface, uint32_t graphicsQFI, uint32_t presentQFI, VkExtent2D appExtent);
	static void cleanUp(VkDevice device);
	
private:

	
    static VkSurfaceFormatKHR chooseSurfaceFormat();
    static VkPresentModeKHR choosePresentMode();
	static VkExtent2D chooseExtent(VkExtent2D appExtent);
	static void createImageViews(VkDevice device);
	
};
