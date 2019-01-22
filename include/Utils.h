#include <iostream>
#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

using namespace std;

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;

	bool operator<(const Vertex& other) const;
};

struct UniformBufferObject {

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

class Utils{

	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
	static void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory& imageMemory);
	static void createImageView(VkImage image, VkImageView &imageView, VkFormat format);
	static void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	static void copyImageToBuffer(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	static void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	
	static VkCommandBuffer beginSingleTimeCommandBuffer();
	static void endSingleTimeCommandBuffer(VkCommandBuffer singleTimeCmdBuffer);
	
	static void createImageSampler(VkSampler &sampler);
	static VkShaderModule createShaderModule(const std::vector<char> &shaderCode);
	static std::vector<char> readFile(const std::string& filename);
	static uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);
	
	static void loadModel(std::string modelFilename, std::vector<Vertex> &vertexArray, std::vector<uint32_t> &indexArray);

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