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
class ComputeApplication{

	static uint32_t OUTPUT_WIDTH;
	static uint32_t OUTPUT_HEIGHT;

    // size of our storage buffer in bytes.
    static VkDeviceSize outputBufferSize; 
	static VkDeviceSize imageSize;

    //input image data
	static unsigned char* inputImageData;

    //In order to use Vulkan, you must create an instance. 
	static VkInstance instance;

    //debug callback
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

	//Image buffer to be exported
	static VkBuffer outputBuffer;
	static VkDeviceMemory outputBufferMemory;

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
    

    //Compute shader used to generate final image, encapsulates shader code
	static VkShaderModule computeShaderModule;

    //The pipeline specifies the pipeline that all graphics and compute commands pass though in Vulkan.
    //We will be creating a simple compute pipeline in this application. 
	static VkPipeline computePipeline;
	static VkPipelineLayout pipelineLayout;
    

    //The command buffer is used to record commands, that will be submitted to a queue.
    //To allocate such command buffers, we use a command pool.
	static VkCommandPool commandPool;
	static VkCommandBuffer mainCommandBuffer;

    
    //used to enable a basic validation layer
	static std::vector<const char *> enabledLayers;

    
    /*In order to execute commands on a device(GPU), the commands must be submitted
    to a queue. The commands are stored in a command buffer, and this command buffer
    is given to the queue. 

    There will be different kinds of queues on the device. For this application, 
    we want a queue that supports compute and graphics operations.*/
	static VkQueue queue;


    //When submitting a command buffer, you must specify to which queue in the family you are submitting to. 
    //This variable keeps track of the index of that queue in its family. 
	static uint32_t queueFamilyIndex;

    

public:

	static void run();

private:

    //Load and saving image
	static void loadImage();

    //app info
	static void createInstance();
	static void findPhysicalDevice();
	static void createDevice();


    // Returns the index of a queue family that supports compute operations. 
	static uint32_t getQueueFamilyIndex();

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

	static void createOutputBuffer();

	static void createOutputImage();
	static void createOutputImageView();
	static void exportOutputImage();

	static void createDescriptorSet();
	static void createComputePipeline();

	static void createMainCommandBuffer();


	static void runCommandBuffer();

	static void cleanup();

	friend class Utils;
};

