#include <iostream>
#include <vulkan/vulkan.h>

#include <array>
#include <vector>
#include <string.h>
#include <assert.h>
#include <stdexcept>
#include <cmath>

const int WIDTH = 3200; // Width of image to render
const int HEIGHT = 2400; // Height of image to render
const int WORKGROUP_SIZE = 32; //Workgroup size in compute shader.

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

using namespace std;
class ComputeApplication{

	struct Pixel {
        float r, g, b, a;
    };

    //In order to use Vulkan, you must create an instance. 
    VkInstance instance;

    //debug callback
    VkDebugReportCallbackEXT debugReportCallback;
    
    //The physical device is some device on the system that supports usage of Vulkan.
    //Often, it is simply a graphics card that supports Vulkan. 
    VkPhysicalDevice physicalDevice;

    //Then we have the logical device VkDevice, which basically allows 
    //us to interact with the physical device. 
    VkDevice device;

    //Compute shader used to generate final image
    VkShaderModule computeShaderModule;

    //The pipeline specifies the pipeline that all graphics and compute commands pass though in Vulkan.
    //We will be creating a simple compute pipeline in this application. 
    VkPipeline computePipeline;
    VkPipelineLayout pipelineLayout;
    

    //The command buffer is used to record commands, that will be submitted to a queue.
    //To allocate such command buffers, we use a command pool.
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    

    //Descriptors provide a way of accessing resources in shaders. They allow us to use 
    //things like uniform buffers, storage buffers and images in GLSL. 
    //A single descriptor represents a single resource, and several descriptors are organized
    //into descriptor sets, which are basically just collections of descriptors.
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;

    //for storage buffer, which will be mapped into CPU memory and exported
    VkBuffer storageBuffer;
    VkDeviceMemory storageBufferMemory;
    
    //Uniform buffer used to pass simple parameters to compute shader
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;

    // size of our storage buffer in bytes.
    uint32_t storageBufferSize; 

    //used to enable a basic validation layer
    std::vector<const char *> enabledLayers;

    
    /*In order to execute commands on a device(GPU), the commands must be submitted
    to a queue. The commands are stored in a command buffer, and this command buffer
    is given to the queue. 

    There will be different kinds of queues on the device. For this application, 
    we want a queue that atleast supports compute operations.*/
    VkQueue computeQueue;


    //When submitting a command buffer, you must specify to which queue in the family you are submitting to. 
    //This variable keeps track of the index of that queue in its family. 
    uint32_t queueFamilyIndex;

    

public:

	void run();

private:
    void saveRenderedImage();

    void createInstance();

    void findPhysicalDevice();

    // Returns the index of a queue family that supports compute operations. 
    uint32_t getComputeQueueFamilyIndex();

    void createDevice();

    // find memory type with desired properties.
    uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);

    void createStorageBuffer();


    void createUniformBuffer(); //work in progress
    void writeUniformBuffer();  //work in progress

    void createDescriptorSetLayout();

    void createDescriptorSet();

    std::vector<char> readFile(const std::string& filename);
    void createComputePipeline();

    void createCommandBuffer();

    void runCommandBuffer();

    void cleanup();
};

//debug callback
VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFn(
        VkDebugReportFlagsEXT                       flags,
        VkDebugReportObjectTypeEXT                  objectType,
        uint64_t                                    object,
        size_t                                      location,
        int32_t                                     messageCode,
        const char*                                 pLayerPrefix,
        const char*                                 pMessage,
        void*                                       pUserData 

);