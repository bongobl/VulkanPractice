#pragma once
#include <iostream>
#include <vulkan/vulkan.h>
#include <fstream>
#include <array>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Vertex.h>

using namespace std;


struct UniformBufferObject {

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	glm::vec3 lightDirection;
	float padding;
	glm::vec3 cameraPosition;
	float padding2;
	glm::vec3 matColor;
	float padding3;
};

class Utils{

	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);

	static void createImage(VkExtent2D dimensions, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
		VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory& imageMemory, bool cubeMapFlag = false);

	static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	static void createImageView(VkImage image, VkImageView &imageView, 
		VkFormat format, VkImageAspectFlags aspectFlags, bool cubeMapFlag = false);

	static void copyBufferToImage(VkBuffer buffer, VkImage image, VkExtent2D imageDimensions, bool cubeMapFlag = false);
	static void copyImageToBuffer(VkBuffer buffer, VkImage image, VkExtent2D imageDimensions);
	static void transitionImageLayout(VkImage image, VkImageLayout oldLayout, 
		VkImageLayout newLayout, bool cubeMapFlag = false);
	
	static VkCommandBuffer beginSingleTimeCommandBuffer();
	static void endSingleTimeCommandBuffer(VkCommandBuffer singleTimeCmdBuffer);
	
	static void createImageSampler(VkSampler &sampler);
	static VkShaderModule createShaderModule(const std::string& filename);
	static std::vector<char> readFile(const std::string& filename);
	static uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);
	
	static void loadModel(std::string modelFilename, std::vector<Vertex> &vertexArray, std::vector<uint32_t> &indexArray);
	static void createImageFromFile(const string imageName, VkImage &image, 
		VkDeviceMemory &imageMemory, VkImageLayout finalLayout);

	static void createCubeMapImageFromFile(const std::vector<string> imageNames, 
		VkImage &image, VkDeviceMemory &imageMemory, VkImageLayout finalLayout);

	static void exportImageAsPNG(VkImage outputImage, VkExtent2D dimensions, std::string fileName, uint32_t numChannels);

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