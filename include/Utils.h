#include <iostream>
#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>
using namespace std;

class Utils{

	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
	static void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory& imageMemory);
	static void createImageView(VkImage image, VkImageView &imageView);
	static void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	static void copyImageToBuffer(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	static void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	
	static VkCommandBuffer beginSingleTimeCommandBuffer();
	static void endSingleTimeCommandBuffer(VkCommandBuffer singleTimeCmdBuffer);
	
	static void createImageSampler(VkSampler &sampler);
	static std::vector<char> readFile(const std::string& filename);
	static uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);
	

	friend class RenderApplication;
};

// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f)                                                                              \
{                                                                                                       \
    VkResult res = (f);                                                                                 \
    if (res != VK_SUCCESS)                                                                              \
    {                                                                                                   \
        printf("Fatal : VkResult is %d in %s at line %d\n", res,  __FILE__, __LINE__); \
        assert(res == VK_SUCCESS);                                                                      \
    }                                                                                                   \
}

//debug callback
VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFunction(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char*                                 pLayerPrefix,
	const char*                                 pMessage,
	void*                                       pUserData

);