#include <SwapChain.h>

void SwapChain::querySupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface){

}

VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(){
    //Implement
	return {};
}

VkPresentModeKHR SwapChain::choosePresentMode(){
    //Implement
	return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

VkExtent2D SwapChain::chooseExtent(){
    //Implement
	return {};
}