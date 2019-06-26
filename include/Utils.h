#pragma once
#include <iostream>
#include <vulkan/vulkan.h>
#include <fstream>
#include <array>
#include <vector>
#include <utility>
#include <ctime>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Vertex.h>

using namespace std;

struct UniformDataComputeShader {
	glm::vec3 netPosition;	//realTime
	uint32_t numParticles;	//startup

	glm::vec3 padding;
	uint32_t pitch;			//startup
	
	float deltaTime;		//realTime
	float netMass;			//startup
	glm::vec2 padding1;
	

};
struct UniformDataVertexShader {

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	
};

struct UniformDataFragShader {

	glm::vec3 lightDirection;
	float textureParam;
	glm::vec3 cameraPosition;
	float normalMapStrength;
	glm::vec3 matColor;
	float padding;
	glm::mat4 lightVP;
};


// provides frequently used helper functions, shouldn't contain any state variables
class Utils{

public: 

	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);

	static void createImage(VkExtent2D dimensions, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
		VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory& imageMemory, bool cubeMapFlag = false);

	static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	static void copyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, 
		VkImageLayout dstImageLayout, VkExtent2D imageExtent, bool cubeMapFlag = false);
	static void createImageView(VkImage image, VkImageView &imageView, 
		VkFormat format, VkImageAspectFlags aspectFlags, bool cubeMapFlag = false);

	static void copyBufferToImage(VkBuffer buffer, VkImage image, VkExtent2D imageDimensions, bool cubeMapFlag = false);
	static void copyImageToBuffer(VkBuffer buffer, VkImage image, VkExtent2D imageDimensions, bool depthImageFlag = false);
	static void transitionImageLayout(VkImage image, VkImageLayout oldLayout, 
		VkImageLayout newLayout, bool cubeMapFlag = false);
	
	static VkCommandBuffer beginSingleTimeCommandBuffer();
	static void endSingleTimeCommandBuffer(VkCommandBuffer singleTimeCmdBuffer);
	
	static void createImageSampler(VkSampler &sampler);
	static VkShaderModule createShaderModule(const std::string& filename);
	static std::vector<char> readFile(const std::string& filename);
	static uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);
	
	static void loadModel(std::string modelFilename, std::vector<Vertex> &vertexArray, std::vector<uint32_t> &indexArray, bool positionsOnly = false);
	static void createImageFromFile(const string imageName, VkImage &image, 
		VkDeviceMemory &imageMemory, VkImageLayout finalLayout);

	static void createCubeMapImageFromFile(const std::vector<string> imageNames, 
		VkImage &image, VkDeviceMemory &imageMemory, VkImageLayout finalLayout);

	//Note: Images expected to be in the VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL layout with VK_FORMAT_B8G8R8A8_UNORM format
	static void exportImageAsPNG(VkImage outputImage, VkExtent2D dimensions, std::string fileName, uint32_t numChannels);

	//Note: Images expected to be in the VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL layout with VK_FORMAT_D32_SFLOAT format
	static void exportDepthImageAsPNG(VkImage depthImage, VkExtent2D dimensions, std::string fileName);

	static void calcTrackBallDeltas(glm::vec2 mousePosition, glm::vec2 prevMousePosition, VkExtent2D appExtent,
		glm::vec3 &rotationAxis, float &rotationAngle);
	static glm::vec3 trackBallMap(glm::vec2 mousePosition, VkExtent2D appExtent);
	static float getRandomFloat(float min, float max);
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