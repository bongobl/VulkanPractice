#include <SwapChain.h>

VkSurfaceCapabilitiesKHR SwapChain::availableSurfaceCapabilities;
std::vector<VkSurfaceFormatKHR> SwapChain::availableSurfaceFormats;
std::vector<VkPresentModeKHR> SwapChain::availableSurfacePresentModes;
VkSurfaceFormatKHR SwapChain::surfaceFormat;
VkPresentModeKHR SwapChain::presentMode;
VkExtent2D SwapChain::extent;
VkSwapchainKHR SwapChain::vulkanHandle;
std::vector<VkImage> SwapChain::images;
std::vector<VkImageView> SwapChain::imageViews;

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

void SwapChain::init(GLFWwindow* window, VkDevice device, VkSurfaceKHR surface, uint32_t graphicsQFI, uint32_t presentQFI, VkExtent2D appExtent){
    
	//make sure we have swapchain support
	if(!hasAdequateSupport()){
		throw std::runtime_error("SwapChain class reported inadequate support, make sure SwapChain::querySupportDetails() was called before initializing a SwapChain object");
	}
	
	surfaceFormat = chooseSurfaceFormat();
	presentMode = choosePresentMode();
	extent = chooseExtent(appExtent);

	//choose minimum number of images 
	uint32_t imageCount = availableSurfaceCapabilities.minImageCount + 1;
	if (availableSurfaceCapabilities.maxImageCount > 0 && imageCount > availableSurfaceCapabilities.maxImageCount) {
		imageCount = availableSurfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;


	if (graphicsQFI != presentQFI) {
		uint32_t bothIndices[] = { graphicsQFI, presentQFI };
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = bothIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
	}

	createInfo.preTransform = availableSurfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	//create vulkan swapchain object
	VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &createInfo, NULL, &vulkanHandle));

	//retrieve swapchain images
	vkGetSwapchainImagesKHR(device, vulkanHandle, &imageCount, NULL);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device, vulkanHandle, &imageCount, images.data());

	//create image views for each swapchain image
	createImageViews(device);
}
void SwapChain::cleanUp(VkDevice device) {

	for (unsigned int i = 0; i < imageViews.size(); ++i) {
		vkDestroyImageView(device, imageViews[i], NULL);
	}

	vkDestroySwapchainKHR(device, vulkanHandle, NULL);
}

VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat() {

	if (availableSurfaceFormats.size() == 1 && availableSurfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (unsigned int i = 0; i < availableSurfaceFormats.size(); ++i) {
		if (availableSurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
			availableSurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableSurfaceFormats[i];
		}
	}

	return availableSurfaceFormats[0];
}

VkPresentModeKHR SwapChain::choosePresentMode() {

	VkPresentModeKHR bestAvailableMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availableSurfacePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestAvailableMode = availablePresentMode;
		}
	}
	return bestAvailableMode;
}

VkExtent2D SwapChain::chooseExtent(VkExtent2D appExtent){

	if (availableSurfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return availableSurfaceCapabilities.currentExtent;
	}

	VkExtent2D minExtent = availableSurfaceCapabilities.minImageExtent;
	VkExtent2D maxExtent = availableSurfaceCapabilities.maxImageExtent;

	VkExtent2D actualExtent;
	actualExtent.width = std::max(minExtent.width, std::min(maxExtent.width, appExtent.width));
	actualExtent.height = std::max(minExtent.height, std::min(maxExtent.height, appExtent.height));

	return actualExtent;
}

void SwapChain::createImageViews(VkDevice device) {

	imageViews.resize(images.size());
	for (unsigned int i = 0; i < images.size(); ++i) {
		Utils::createImageView(images[i], imageViews[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}