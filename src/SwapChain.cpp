#include <SwapChain.h>

VkSurfaceCapabilitiesKHR SwapChain::availableSurfaceCapabilities;
std::vector<VkSurfaceFormatKHR> SwapChain::availableSurfaceFormats;
std::vector<VkPresentModeKHR> SwapChain::availableSurfacePresentModes;

void SwapChain::querySupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {

	//make sure support details are empty before populating them
	availableSurfaceCapabilities = {};
	availableSurfaceFormats.clear();
	availableSurfacePresentModes.clear();

	//query supported capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &availableSurfaceCapabilities);

	//query supported formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);
	if (formatCount != 0) {
		availableSurfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, availableSurfaceFormats.data());
	}

	//query supported present modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
	if (presentModeCount != 0) {
		availableSurfacePresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, availableSurfacePresentModes.data());
	}
}

bool SwapChain::hasAdequateSupport() {
	return !availableSurfaceFormats.empty() && !availableSurfacePresentModes.empty();
}

void SwapChain::init(GLFWwindow* window, VkDevice device, VkSurfaceKHR surface, QueueFamilyMap queueFamilyMap, VkExtent2D appExtent){
    
	//make sure we have swapchain support
	if(!hasAdequateSupport()){
		throw std::runtime_error("SwapChain class reported inadequate support, make sure SwapChain::querySupportDetails() was called before initializing a SwapChain object");
	}
}




VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat() const{

	if (availableSurfaceFormats.size() == 1 && availableSurfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (unsigned int i = 0; i < availableSurfaceFormats.size(); ++i) {
		if (availableSurfaceFormats[i].format == VK_FORMAT_R8G8B8A8_UNORM &&
			availableSurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableSurfaceFormats[i];
		}
	}

	return availableSurfaceFormats[0];
}

VkPresentModeKHR SwapChain::choosePresentMode() const{
    //Implement
	return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

VkExtent2D SwapChain::chooseExtent(VkExtent2D appExtent) const{
    //Implement
	return {};
}