#include <iostream>
#include <vulkan/vulkan.h>

#include <array>
#include <vector>
#include <string.h>
#include <assert.h>
#include <stdexcept>
#include <cmath>
#include <string>
using namespace std;

const int WORKGROUP_SIZE = 32; //Workgroup size in compute shader.

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

using namespace std;
class RenderApplication{

	static uint32_t IMAGE_WIDTH;
	static uint32_t IMAGE_HEIGHT;

    // size of our storage image in bytes.
	static VkDeviceSize imageSize;

    //input image data
	static unsigned char* inputImageData;

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


	//Input image
	static VkImage inputImage;
	static VkDeviceMemory inputImageMemory;
	static VkImageView inputImageView;

    //Uniform buffer used to pass simple parameters to compute shader
	static VkBuffer uniformBuffer;
	static VkDeviceMemory uniformBufferMemory;

	//Output Image
	static VkImage outputImage;
	static VkDeviceMemory outputImageMemory;
	static VkImageView outputImageView;
	 
    //Descriptors provide a way of accessing resources in shaders. They allow us to use 
    //things like uniform buffers, storage buffers and images in GLSL. 
    //A single descriptor represents a single resource, and several descriptors are organized
    //into descriptor sets, which are basically just collections of descriptors.
	static VkDescriptorPool descriptorPool;
	static VkDescriptorSet descriptorSet;
	static VkDescriptorSetLayout descriptorSetLayout;
    

    //The pipeline specifies the pipeline that all graphics and compute commands pass though in Vulkan.
    //We will be creating a simple compute pipeline in this application. 
	static VkPipeline computePipeline;
	static VkPipelineLayout pipelineLayout;
    
	//For graphics
	static VkRenderPass renderPass;
	static VkPipeline graphicsPipeline;

    //The command buffer is used to record commands, that will be submitted to a queue.
    //To allocate such command buffers, we use a command pool.
	static VkCommandPool commandPool;
	static VkCommandBuffer mainCommandBuffer;


	//Index of queue family which supports our desired features, in our case, graphics and compute
	static uint32_t queueFamilyIndex;

	//Queue for graphics and compute operation
	static VkQueue queue;

	//The layers and instance extensions required for our application
	const static std::vector<const char *> requiredLayers;
	const static std::vector<const char *> requiredInstanceExtensions;



public:

	static void run();

private:

    //Load and saving image
	static void loadImage();

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
	static void createInputImage();
	static void writeToInputImage();
	static void createInputImageView();

	static void createUniformBuffer();
	static void writeToUniformBuffer();

	static void createOutputImage();
	static void createOutputImageView();
	static void exportOutputImage();

	static void createDescriptorSet();
	static void createComputePipeline();

	//to start rendering
	static void createRenderPass();
	static void createGraphicsPipeline();

	static void createMainCommandBuffer();


	static void runCommandBuffer();

	static void cleanup();

	friend class Utils;
};

