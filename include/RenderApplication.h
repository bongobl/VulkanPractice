#pragma once


#include <iostream>
#include <array>
#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <cmath>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <Utils.h>
#include <QueueFamilyMap.h>
#include <SwapChain.h>
#include <Lighting.h>
using namespace std;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#define MAX_FRAME_RATE 60

class RenderApplication{

	//test glfw window
	static GLFWwindow* window;
	static bool windowResized;

	//width and height of render area
	static VkExtent2D desiredInitialExtent;
	
	//Structures to hold all of our app's requirements
	static std::vector<const char*> requiredInstanceLayers;
	static std::vector<const char*> requiredInstanceExtensions;
	static std::vector<const char*> requiredDeviceExtensions;
	static VkPhysicalDeviceFeatures requiredDeviceFeatures;


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


	//Queue handles for our operations
	static VkQueue graphicsQueue;
	static VkQueue transferQueue;
	static VkQueue presentQueue;

	//handle to the window surface that we will present rendered images to
	static VkSurfaceKHR surface;

	//Used to store mesh data as they are loaded in from disk
	static std::vector<Vertex> vertexArray;
	static std::vector<uint32_t> indexArray;
	
	//used to store all the vertices
	static VkBuffer vertexBuffer;
	static VkDeviceMemory vertexBufferMemory;

	//used to store all the indices
	static VkBuffer indexBuffer;
	static VkDeviceMemory indexBufferMemory;

    //Uniform buffers used to pass simple parameters shaders
	static std::vector<VkBuffer> tessShaderUBOs;
	static std::vector<VkDeviceMemory> tessShaderUBOMemories;
	static std::vector<VkBuffer> fragShaderUBOs;
	static std::vector<VkDeviceMemory> fragShaderUBOMemories;

	//for our displacement texture
	static VkImage displacementMap;
	static VkDeviceMemory displacementMapMemory;
	static VkImageView displacementMapView;

	//for our diffuse texture
	static VkImage diffuseTexture;
	static VkDeviceMemory diffuseTextureMemory;
	static VkImageView diffuseTextureView;

	//for our normal texture
	static VkImage normalTexture;
	static VkDeviceMemory normalTextureMemory;
	static VkImageView normalTextureView;

	//for our environment map
	static VkImage environmentMap;
	static VkDeviceMemory environmentMapMemory;
	static VkImageView environmentMapView;

	//to sample image textures
	static VkSampler imageSampler;

	//Depth attachment image to use
	static VkImage depthAttachmentImage;
	static VkDeviceMemory depthAttachmentImageMemory;
	static VkImageView depthAttachmentImageView;

    //Structures for Descriptor creation
	static VkDescriptorPool descriptorPool;	
	static VkDescriptorSetLayout descriptorSetLayout;
    static std::vector<VkDescriptorSet> descriptorSets;

	//our render pass with one color attachment
	static VkRenderPass renderPass;

	//frame buffers to reference our swapchain images
	static std::vector<VkFramebuffer> swapChainFrameBuffers;

    //The pipeline specifies the pipeline that all graphics and compute commands pass though in Vulkan.
	static VkPipelineLayout pipelineLayout;
	static VkPipeline graphicsPipeline;
    
	//realtime rendering and presenting
	static std::vector<VkFence> inFlightFences;
	static std::vector<VkSemaphore> imageAvailableSemaphores;
	static std::vector<VkSemaphore> renderFinishedSemaphores;
	static std::vector<VkSemaphore> shadowMapDoneSemaphores;
	static int currentFrame;

    //The command buffer is used to record commands, that will be submitted to a queue.
    //To allocate such command buffers, we use a command pool.
	static VkCommandPool graphicsCommandPool;

	static std::vector<VkCommandBuffer> renderCommandBuffers;

	//SCENE VARIABLES

	//time variables
	static float currTime;
	static float prevTime;
	static float deltaTime;

	//mouse
	static glm::vec2 mousePosition;
	static glm::vec2 prevMousePosition;
	static bool isLeftMouseButtonDown;
	static bool isRightMouseButtonDown;

	//model rotation
	static glm::mat4 modelCorrect;
	static glm::vec3 modelSpinAxis;
	static float modelSpinAngle;
	static glm::mat4 modelOrientation;

	//light rotation;
	static glm::mat3 lightOrientation;

public:

	static void run();

private:

	//high level app functions
	static void initGLFWWindow();
	static void createAllVulkanResources();
	static void initScene();
	static void drawFrame();
	static void updateScene();
	static void cleanup();

	//GLFW callbacks
	static void windowResizeCallback(GLFWwindow* resizedWindow, int newWidth, int newHeight);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorMovedCallback(GLFWwindow* window, double xpos, double ypos);

	
	static VkExtent2D waitToGetNonZeroWindowExtent();	//continuously loops until window width/height are non-zero, then returns the extent

	static int getPrevFrameIndex(int currFrameIndex);

    //specify all required instance layers, instance extensions and device extensions here
    static void configureAllRequirements();

	//app info
	static void createInstance();
	static void createSurface();
	static void findPhysicalDevice();
	static void createDevice();
	static void createSwapChain(const VkExtent2D appExtent);
	static void recreateAppExtentDependents();
	
	//helper to find physical device
	static bool isValidPhysicalDevice(VkPhysicalDevice potentialPhysicalDevice);

	
    //layouts and pools
	static void createDescriptorSetLayout();
	static void createDescriptorPool();
	static void createCommandPool();

	//GPU resources
	static void loadVertexAndIndexArrays();
	static void createVertexBuffer();
    static void writeToVertexBuffer();

    static void createIndexBuffer();
    static void writeToIndexBuffer();

	static void createUniformBuffers();
	static void writeToUniformBuffer(uint32_t imageIndex);

	static void createDisplacementMap();
	static void createDisplacementMapView();

	static void createDiffuseTexture();
	static void createDiffuseTextureView();

	static void createNormalTexture();
	static void createNormalTextureView();

	static void createEnvironmentMap();
	static void createEnvironmentMapView();

	static void createImageSampler();

	static void createDepthAttachmentImage();
	static void createDepthAttachmentImageView();

	
	static void createDescriptorSets();


	static void createRenderPass();	
	static void createSwapChainFrameBuffers();
	static void createGraphicsPipeline();
	static void createSyncObjects();

	//the command buffer that will render the scene to our swapchain images
	static void createRenderCommandBuffers();

	static void updateModelRotation();
	static void updateLightRotation();

	//to allow Utils class to perform transfer operations
	static VkCommandPool getTransferCmdPool();
	static VkQueue getTransferQueue();
	
	//for shadow map
	static VkCommandPool getGraphicsCmdPool();
	static VkQueue getGraphicsQueue();
	static VkSampler getImageSampler();
	static VkImageView getDisplacementMapView();
	
	friend class Utils;
	friend class Lighting;
};
