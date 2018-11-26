#include <iostream>
#include <vulkan/vulkan.h>

#include <array>
#include <vector>
#include <string.h>
#include <assert.h>
#include <stdexcept>
#include <cmath>

const int WIDTH = 3200; // Size of rendered mandelbrot set.
const int HEIGHT = 2400; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 32; // Workgroup size in compute shader.

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

    /*
    In order to use Vulkan, you must create an instance. 
    */
    VkInstance instance;

    VkDebugReportCallbackEXT debugReportCallback;
    /*
    The physical device is some device on the system that supports usage of Vulkan.
    Often, it is simply a graphics card that supports Vulkan. 
    */
    VkPhysicalDevice physicalDevice;
    /*
    Then we have the logical device VkDevice, which basically allows 
    us to interact with the physical device. 
    */
    VkDevice device;

    /*
    The pipeline specifies the pipeline that all graphics and compute commands pass though in Vulkan.

    We will be creating a simple compute pipeline in this application. 
    */
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkShaderModule computeShaderModule;

    /*
    The command buffer is used to record commands, that will be submitted to a queue.

    To allocate such command buffers, we use a command pool.
    */
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    /*

    Descriptors represent resources in shaders. They allow us to use things like
    uniform buffers, storage buffers and images in GLSL. 

    A single descriptor represents a single resource, and several descriptors are organized
    into descriptor sets, which are basically just collections of descriptors.
    */
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;

    /*
    The mandelbrot set will be rendered to this buffer.

    The memory that backs the buffer is bufferMemory. 
    */
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
        
    uint32_t bufferSize; // size of `buffer` in bytes.

    std::vector<const char *> enabledLayers;

    /*
    In order to execute commands on a device(GPU), the commands must be submitted
    to a queue. The commands are stored in a command buffer, and this command buffer
    is given to the queue. 

    There will be different kinds of queues on the device. Not all queues support
    graphics operations, for instance. For this application, we at least want a queue
    that supports compute operations. 
    */
    VkQueue queue; // a queue supporting compute operations.

    /*
    Groups of queues that have the same capabilities(for instance, they all supports graphics and computer operations),
    are grouped into queue families. 
    
    When submitting a command buffer, you must specify to which queue in the family you are submitting to. 
    This variable keeps track of the index of that queue in its family. 
    */
    uint32_t queueFamilyIndex;

    //for our uniform buffer, in progress
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;

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

    void createBuffer();


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