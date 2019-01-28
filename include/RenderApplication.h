#pragma once
#include <iostream>
#include <vulkan/vulkan.h>
#include <array>
#include <vector>
#include <string.h>
#include <assert.h>
#include <stdexcept>
#include <cmath>
#include <string>
#include <Utils.h>
using namespace std;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


class RenderApplication{


	//width and height
	static VkExtent2D resolution;

    //In order to use Vulkan, you must create an instance. 
	static VkInstance instance;

    //object to store debug callback
	static VkDebugReportCallbackEXT debugReportCallback;
    
    //The physical device is some device on the system that supports usage of Vulkan.
    //Often, it is simply a graphics card that supports Vulkan. 
	static VkPhysicalDevice physicalDevice;

    //Then we have the logical device VkDevice, which basically allows 
    //us to interact with the physical device. 
	static VkDevice device;

	//used to store all the vertices
	static VkBuffer vertexBuffer;
	static VkDeviceMemory vertexBufferMemory;

	//used to store all the indices
	static VkBuffer indexBuffer;
	static VkDeviceMemory indexBufferMemory;


    //Uniform buffer used to pass simple parameters to compute shader
	static VkBuffer uniformBuffer;
	static VkDeviceMemory uniformBufferMemory;

	//Color attachment image to render to
	static VkImage colorImage;
	static VkDeviceMemory colorImageMemory;
	static VkImageView colorImageView;

	

    //Descriptors provide a way of accessing resources in shaders. They allow us to use 
    //things like uniform buffers, storage buffers and images in GLSL. 
    //A single descriptor represents a single resource, and several descriptors are organized
    //into descriptor sets, which are basically just collections of descriptors.
	static VkDescriptorPool descriptorPool;
	static VkDescriptorSet descriptorSet;
	static VkDescriptorSetLayout descriptorSetLayout;
    
	//our render pass with one color attachment
	static VkRenderPass renderPass;

	//frame buffer to reference our color attachment image
	static VkFramebuffer frameBuffer;

    //The pipeline specifies the pipeline that all graphics and compute commands pass though in Vulkan.
	static VkPipelineLayout pipelineLayout;
	static VkPipeline graphicsPipeline;
	
    
	
    //The command buffer is used to record commands, that will be submitted to a queue.
    //To allocate such command buffers, we use a command pool.
	static VkCommandPool graphicsCommandPool;
	static VkCommandBuffer mainCommandBuffer;


	//Index of queue family which supports our desired features, in our case, graphics
	static uint32_t graphicsQueueFamilyIndex;

	//Queue for graphics and compute operation
	static VkQueue graphicsQueue;

	//The layers and instance extensions required for our application
	const static std::vector<const char *> requiredLayers;
	const static std::vector<const char *> requiredInstanceExtensions;



public:

	static void run();

private:


    //app info
	static void createInstance();
	static void findPhysicalDevice();
	static void createDevice();

	
	//helper to find physical device
	static bool isValidPhysicalDevice(VkPhysicalDevice potentialPhysicalDevice, uint32_t &familyIndex);

    // Returns the index of a queue family that supports compute operations. 
	static uint32_t getQueueFamilyIndex(VkPhysicalDevice currPhysicalDevice);

	

    //layouts and pools
	static void createDescriptorSetLayout();
	static void createDescriptorPool();
	static void createCommandPool();

	//GPU resources
	static void createVertexBuffer();
    static void writeToVertexBuffer();

    static void createIndexBuffer();
    static void writeToIndexBuffer();

	static void createUniformBuffer();
	static void writeToUniformBuffer();
	
	static void createColorImage();
	static void createColorImageView();

	

	static void createDescriptorSet();

	//to start rendering
	static void createRenderPass();
	static void createFrameBuffer();
	static void createGraphicsPipeline();

	static void createMainCommandBuffer();

	static void runMainCommandBuffer();

	static void exportAsImage();

	static void cleanup();

	//to allow Utils class to perform transfer operations
	static VkCommandPool& getTransferCmdPool();
	static VkQueue& getTransferQueue();
	
	friend class Utils;
};

