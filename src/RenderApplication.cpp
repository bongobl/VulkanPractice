#include <RenderApplication.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define MAX_FRAMES_IN_FLIGHT 2

//Initialize static members
GLFWwindow* RenderApplication::window;
VkExtent2D RenderApplication::desiredIntialExtent = {1920,1470};
std::vector<const char*> RenderApplication::requiredInstanceLayers;
std::vector<const char*> RenderApplication::requiredInstanceExtensions;
std::vector<const char*> RenderApplication::requiredDeviceExtensions;
VkPhysicalDeviceFeatures RenderApplication::requiredDeviceFeatures = {};
QueueFamilyMap RenderApplication::queueFamilyIndices;


VkInstance RenderApplication::instance;
VkDebugReportCallbackEXT RenderApplication::debugReportCallback;
VkPhysicalDevice RenderApplication::physicalDevice;
VkDevice RenderApplication::device;
VkQueue RenderApplication::graphicsQueue;
VkQueue RenderApplication::transferQueue;
VkQueue RenderApplication::presentQueue;
VkSurfaceKHR RenderApplication:: surface;
std::vector<Vertex> RenderApplication::vertexArray;
std::vector<uint32_t> RenderApplication::indexArray;
VkBuffer RenderApplication::vertexBuffer;
VkDeviceMemory RenderApplication::vertexBufferMemory;
VkBuffer RenderApplication::indexBuffer;
VkDeviceMemory RenderApplication::indexBufferMemory;
std::vector<VkBuffer> RenderApplication::tessShaderUBOs;
std::vector<VkDeviceMemory> RenderApplication::tessShaderUBOMemories;
std::vector<VkBuffer> RenderApplication::fragShaderUBOs;
std::vector<VkDeviceMemory> RenderApplication::fragShaderUBOMemories;
VkImage RenderApplication::diffuseTexture;
VkDeviceMemory RenderApplication::diffuseTextureMemory;
VkImageView RenderApplication::diffuseTextureView;
VkImage RenderApplication::environmentMap;
VkDeviceMemory RenderApplication::environmentMapMemory;
VkImageView RenderApplication::environmentMapView;
VkSampler RenderApplication::textureSampler;
VkImage RenderApplication::depthAttachmentImage;
VkDeviceMemory RenderApplication::depthAttachmentImageMemory;
VkImageView RenderApplication::depthAttachmentImageView;
std::vector<VkFramebuffer> RenderApplication::swapChainFrameBuffers;
VkDescriptorPool RenderApplication::descriptorPool;
std::vector<VkDescriptorSet> RenderApplication::descriptorSets;
VkDescriptorSetLayout RenderApplication::descriptorSetLayout;
VkRenderPass RenderApplication::renderPass;
VkPipelineLayout RenderApplication::pipelineLayout;
VkPipeline RenderApplication::graphicsPipeline;
std::vector<VkFence> RenderApplication::inFlightFences;
std::vector<VkSemaphore> RenderApplication::imageAvailableSemaphores;
std::vector<VkSemaphore> RenderApplication::renderFinishedSemaphores;
int RenderApplication::currentFrame;
VkCommandPool RenderApplication::graphicsCommandPool;
std::vector<VkCommandBuffer> RenderApplication::renderCommandBuffers;
float RenderApplication::modelRotation = 0;

void RenderApplication::run() {


	//init glfw window
	initGLFWWindow();

	//create all vulkan resources we will need in the app
	createAllVulkanResources();

	//copyOutputToSwapChainImages();

	//play around with window as long as we want
	cout << "In Main Loop" << endl;
	currentFrame = 0;	//set beginning frame to work with
	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
		mainLoop();
	}

	//wait for remaining images to finish rendering and presenting
	vkDeviceWaitIdle(device);

    // Clean up all resources.
    cleanup();

	cout << "Successful Close" << endl;
}

void RenderApplication::initGLFWWindow(){

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(desiredIntialExtent.width,desiredIntialExtent.height, "Test Window", NULL, NULL);
	glfwSetFramebufferSizeCallback(window, &frameBufferResizeCallback);
}

void RenderApplication::frameBufferResizeCallback(GLFWwindow* resizedWindow, int newWidth, int newHeight){
	cout << "Window resized" << endl;

	
}

void RenderApplication::createAllVulkanResources() {

	//add all requirements that this app will need from the device and instance
	configureAllRequirements();

	// Initialize vulkan
	createInstance();
	createSurface();
	findPhysicalDevice();
	createDevice();
	createSwapChain();

	//create descriptor and command resources
	createDescriptorSetLayout();
	createDescriptorPool();
	createCommandPool();


	//create all device data
	loadVertexAndIndexArrays();
	createVertexBuffer();
	writeToVertexBuffer();

	createIndexBuffer();
	writeToIndexBuffer();

	createUniformBuffers();

	for(unsigned int i = 0; i < SwapChain::images.size(); ++i){
		writeToUniformBuffer(i);
	}

	createDiffuseTexture();
	createDiffuseTextureView();

	createEnvironmentMap();
	createEnvironmentMapView();

	createTextureSampler();

	createDepthAttachmentImage();
	createDepthAttachmentImageView();

	//create descriptors
	createDescriptorSets();

	//create rendering utils
	createRenderPass();	//will rename
	createSwapChainFrameBuffers();
	createGraphicsPipeline();
	createSyncObjects();

	//record command buffer
	createRenderCommandBuffers();

}

void RenderApplication::mainLoop(){
	
	uint64_t MAX_UNSIGNED_64_BIT_VAL = std::numeric_limits<uint64_t>::max();

	VkResult result;
	uint32_t imageIndex;

	//wait for image at current frame to finish rendering
	vkWaitForFences(device,1, &inFlightFences[currentFrame], VK_TRUE, MAX_UNSIGNED_64_BIT_VAL);

	//acquire next image from swapchain, which may currently still be presenting
	result = vkAcquireNextImageKHR(
		device, 
		SwapChain::vulkanHandle, 
		MAX_UNSIGNED_64_BIT_VAL,
		imageAvailableSemaphores[currentFrame],
		VK_NULL_HANDLE,
		&imageIndex
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		throw std::runtime_error("Swapchain reported out of date after acquiring next image, need to recreate");
	}
	VK_CHECK_RESULT(result);

	//update uniform data for this frame
	writeToUniformBuffer(imageIndex);

	//Enqueue render to this image
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &renderCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]));

	//Enqueue presenting this image
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &SwapChain::vulkanHandle;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Swapchain reported out of date or suboptimal after present queue submit, need to recreate");
	}
	VK_CHECK_RESULT(result);
	
}
void RenderApplication::cleanup() {

	//clean up all Vulkan resources
	if (enableValidationLayers) {
		// destroy callback.
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func == nullptr) {
			throw std::runtime_error("Error: Could not load vkDestroyDebugReportCallbackEXT");
		}
		func(instance, debugReportCallback, NULL);
	}

	//free vertex buffer
	vkDestroyBuffer(device, vertexBuffer, NULL);
	vkFreeMemory(device, vertexBufferMemory, NULL);

	//free index buffer
	vkDestroyBuffer(device, indexBuffer, NULL);
	vkFreeMemory(device, indexBufferMemory, NULL);

	//free uniform buffers
	for(unsigned int i = 0; i < SwapChain::images.size(); ++i){
		vkDestroyBuffer(device, tessShaderUBOs[i], NULL);
		vkFreeMemory(device, tessShaderUBOMemories[i], NULL);
		vkDestroyBuffer(device, fragShaderUBOs[i], NULL);
		vkFreeMemory(device, fragShaderUBOMemories[i], NULL);
	}

	//free diffuse texture
	vkDestroyImageView(device, diffuseTextureView, NULL);
	vkDestroyImage(device, diffuseTexture, NULL);
	vkFreeMemory(device, diffuseTextureMemory, NULL);

	//free environment map texture
	vkDestroyImageView(device, environmentMapView, NULL);
	vkDestroyImage(device, environmentMap, NULL);
	vkFreeMemory(device, environmentMapMemory, NULL);

	//free sampler
	vkDestroySampler(device, textureSampler, NULL);


	//free depth attachment image
	vkDestroyImageView(device, depthAttachmentImageView, NULL);
	vkDestroyImage(device, depthAttachmentImage, NULL);
	vkFreeMemory(device, depthAttachmentImageMemory, NULL);

	for (unsigned int i = 0; i < SwapChain::images.size(); ++i) {
		vkDestroyFramebuffer(device, swapChainFrameBuffers[i], NULL);
	}

	vkDestroyRenderPass(device, renderPass, NULL);
	vkDestroyDescriptorPool(device, descriptorPool, NULL);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);

	SwapChain::cleanUp(device);
	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i){
		vkDestroyFence(device, inFlightFences[i], NULL);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);
	}
	vkDestroyPipeline(device, graphicsPipeline, NULL);
	vkDestroyPipelineLayout(device, pipelineLayout, NULL);
	vkDestroyCommandPool(device, graphicsCommandPool, NULL);
	vkDestroyDevice(device, NULL);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, NULL);

	glfwDestroyWindow(window);
	glfwTerminate();

}

void RenderApplication::configureAllRequirements(){


	//If validation layers enabled
	if(enableValidationLayers){
		cout << "<Debug Mode: Validation Layers Enabled>" << endl;

		//our validation requires one instance layer and one instance extension
		requiredInstanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		requiredInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	//add glfw instance extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for(unsigned int i = 0; i < glfwExtensionCount; ++i){
		requiredInstanceExtensions.push_back(glfwExtensions[i]);
	}


	//specify required device features 
	requiredDeviceFeatures.samplerAnisotropy = VK_TRUE;
	requiredDeviceFeatures.tessellationShader = VK_TRUE;
	requiredDeviceFeatures.fillModeNonSolid = VK_TRUE;
	requiredDeviceFeatures.wideLines = VK_TRUE;

	//specify what queue capabilities we need
	queueFamilyIndices.addRequiredQueueType(VK_QUEUE_GRAPHICS_BIT);
	queueFamilyIndices.addRequiredQueueType(VK_QUEUE_TRANSFER_BIT);
	queueFamilyIndices.addRequiredQueueType(ADDITIONAL_VK_QUEUE_PRESENT_BIT);

	//swap chain device extension
	requiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}
void RenderApplication::createInstance() {


	//Check for presence of required instance layers
	uint32_t numAvailableLayers;

	vkEnumerateInstanceLayerProperties(&numAvailableLayers, NULL);
	std::vector<VkLayerProperties> allAvailableInstanceLayers(numAvailableLayers);
	vkEnumerateInstanceLayerProperties(&numAvailableLayers, allAvailableInstanceLayers.data());

	for (const char* currRequiredLayer : requiredInstanceLayers) {

		bool foundRequiredLayer = false;
		for (VkLayerProperties currAvailableLayerProp : allAvailableInstanceLayers) {
			if (strcmp(currRequiredLayer, currAvailableLayerProp.layerName) == 0) {
				foundRequiredLayer = true;
				break;
			}
		}
		if (!foundRequiredLayer) {
			string errorMessage = "Instance Layer " + string(currRequiredLayer) + " not supported";
			throw std::runtime_error(errorMessage);
		}
	}


	//check for presence of required instance extensions
    uint32_t numAvailableExtensions;

    vkEnumerateInstanceExtensionProperties(NULL, &numAvailableExtensions, NULL);
    std::vector<VkExtensionProperties> allAvailableExtensions(numAvailableExtensions);
    vkEnumerateInstanceExtensionProperties(NULL, &numAvailableExtensions, allAvailableExtensions.data());


	for (const char* currRequiredExtension : requiredInstanceExtensions) {

		bool foundRequiredExtension = false;
		for (VkExtensionProperties currAvailableExtension : allAvailableExtensions) {
			if (strcmp(currRequiredExtension, currAvailableExtension.extensionName) == 0) {
				foundRequiredExtension = true;
				break;
			}
		}
		if (!foundRequiredExtension) {
			string errorMessage = "Instance Extension " + string(currRequiredExtension) + " not supported";
			throw std::runtime_error(errorMessage);
		}
	}

    //Next, we actually create the instance.

    //Contains application info. This is actually not that important.
    //The only real important field is apiVersion.
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Vulkan Render Application";
    applicationInfo.applicationVersion = 0;
    applicationInfo.pEngineName = "RenderEngine";
    applicationInfo.engineVersion = 0;
    applicationInfo.apiVersion = VK_API_VERSION_1_1;;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &applicationInfo;


    // Give our desired instance layers to vulkan.
	createInfo.enabledLayerCount = (uint32_t)requiredInstanceLayers.size();
	createInfo.ppEnabledLayerNames = requiredInstanceLayers.data();

	// Give our desired instance extensions to vulkan
    createInfo.enabledExtensionCount = (uint32_t)requiredInstanceExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();


    //Actually create the instance. Having created the instance, we can actually start using vulkan.
    VK_CHECK_RESULT( vkCreateInstance( &createInfo, NULL, &instance));


    //Register a callback function for the extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME, so that warnings emitted from the validation
    //layer are actually printed.
    if (enableValidationLayers) {
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        createInfo.pfnCallback = &debugReportCallbackFunction;

        // We have to explicitly load this function and have our local function pointer point to it
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        if (vkCreateDebugReportCallbackEXT == nullptr) {
            throw std::runtime_error("Could not load vkCreateDebugReportCallbackEXT");
        }

        // Create and register callback.
        VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(instance, &createInfo, NULL, &debugReportCallback));
    }

}

void RenderApplication::createSurface(){

	VK_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));
}
void RenderApplication::findPhysicalDevice() {

    //In this function, we find a physical device that can be used with Vulkan.
    //So, first we will list all physical devices on the system with vkEnumeratePhysicalDevices .

    uint32_t physicalDeviceCount;

    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
    if (physicalDeviceCount == 0) {
        throw std::runtime_error("could not find a device with vulkan support");
    }

    std::vector<VkPhysicalDevice> allPhysicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, allPhysicalDevices.data());


    for (VkPhysicalDevice currPhysicalDevice : allPhysicalDevices) {

		if (isValidPhysicalDevice(currPhysicalDevice)) {
			physicalDevice = currPhysicalDevice;
			return;
		}
    }

	throw std::runtime_error("Error: Could not load find a valid physical device for our operations");
}

bool RenderApplication::isValidPhysicalDevice(VkPhysicalDevice potentialPhysicalDevice) {

	//For this physical device to be valid, it needs to support our device features, device extensions and have
	//queue families for all our operations (And if using swapchain, supports swapchain operations)

	//Check for support of required device features
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(potentialPhysicalDevice, &supportedFeatures);

	VkBool32* allRequired = (VkBool32*)&requiredDeviceFeatures;
	VkBool32* allSupported = (VkBool32*)&supportedFeatures;
	int numPossibleFeatures = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
	
	for(int i =  0; i < numPossibleFeatures; ++i){

		if(allRequired[i] == VK_TRUE && allSupported[i] == VK_FALSE){
			cout << "Physical Device Feature " << i << " not supported" << endl;
			return false;
		}
	}

    //Check for support of required device extensions
	uint32_t numAvailableExtensions;
	vkEnumerateDeviceExtensionProperties(potentialPhysicalDevice, NULL, &numAvailableExtensions, NULL);
	std::vector<VkExtensionProperties> allAvailableExtensions(numAvailableExtensions);
	vkEnumerateDeviceExtensionProperties(potentialPhysicalDevice, NULL, &numAvailableExtensions, allAvailableExtensions.data());

	for (const char* currRequiredExtension : requiredDeviceExtensions) {

		bool foundRequiredExtension = false;
		for (VkExtensionProperties currAvailableExtension : allAvailableExtensions) {

			if (strcmp(currRequiredExtension, currAvailableExtension.extensionName) == 0) {
				foundRequiredExtension = true;
				break;
			}
		}

		//this physical device doesn't support all our required extensions
		if (!foundRequiredExtension) {
			return false;
		}
	}

	// Does this physical device support swapchain functions for our purposes
	// Note: It is important that we only call querySupportDetails after we check 
	// that this physical device supports our swapchain device extension
	SwapChain::querySupportDetails(potentialPhysicalDevice, surface);
	bool hasSwapChainSupport = SwapChain::hasAdequateSupport();

	bool hasAllIndices = queueFamilyIndices.compute(potentialPhysicalDevice, surface);


	return hasAllIndices && hasSwapChainSupport;

}

void RenderApplication::createDevice() {

	//only need unique queue families
	std::set<uint32_t> uniqueQueueFamilies;
	for (int i = 0; i < queueFamilyIndices.numRequired(); ++i) {
		uniqueQueueFamilies.insert(queueFamilyIndices.getQueueFamilyIndexAt(i));
	}

	//contains all the queue create info structs the device needs to know about
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	
	//for each unique queue family, we need a VkDeviceQueueCreateInfo struct
	for (uint32_t currFamilyIndex : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = currFamilyIndex;
		queueCreateInfo.queueCount = 1; // get one queue from this family. We don't need more.	
		float queuePriorities = 1.0;
		queueCreateInfo.pQueuePriorities = &queuePriorities;
		queueCreateInfos.push_back(queueCreateInfo);
	}


    // Now we create the logical device. The logical device allows us to interact with the physical device.
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	// specify queue info
	deviceCreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	
	// specify the required features
    deviceCreateInfo.pEnabledFeatures = &requiredDeviceFeatures;

	// need to specify instance layers here as well.
	deviceCreateInfo.enabledLayerCount = (uint32_t)requiredInstanceLayers.size();  
	deviceCreateInfo.ppEnabledLayerNames = requiredInstanceLayers.data();

	// give our required device extensions to vulkan
	deviceCreateInfo.enabledExtensionCount = (uint32_t)requiredDeviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

    VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device)); // create logical device.

	//the index within this queue family of the queue to retrieve, we just take the first one
    uint32_t particularQueueIndex = 0;

    // Get a handle to the first member of the queue family.
    vkGetDeviceQueue(device, queueFamilyIndices.getQueueFamilyIndexAt(0), particularQueueIndex, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.getQueueFamilyIndexAt(1), particularQueueIndex, &transferQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.getQueueFamilyIndexAt(2), particularQueueIndex, &presentQueue);
}

void RenderApplication::createSwapChain() {

	int actualWidth, actualHeight;
	glfwGetFramebufferSize(window, &actualWidth, &actualHeight);

	VkExtent2D actualWindowExtent = {
		(uint32_t)actualWidth,
		(uint32_t)actualHeight
	};

	SwapChain::init(
		device,			//App logical device
		surface,		//Vulkan Surface object
		queueFamilyIndices.getQueueFamilyIndexAt(0),	//graphics queue family index
		queueFamilyIndices.getQueueFamilyIndexAt(1),	//present queue family index
		actualWindowExtent								//actual extent of the window
	);

}

void RenderApplication::loadVertexAndIndexArrays(){
	Utils::loadModel("resources/models/Heptoroid.obj", vertexArray, indexArray);
}
void RenderApplication::createVertexBuffer(){

	VkDeviceSize vertexArraySize = vertexArray.size() * sizeof(Vertex);
	Utils::createBuffer(
		vertexArraySize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer, vertexBufferMemory
	);

}

void RenderApplication::writeToVertexBuffer(){

	VkDeviceSize vertexArraySize = vertexArray.size() * sizeof(Vertex);

	//create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;


	Utils::createBuffer(
		vertexArraySize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		stagingBuffer, stagingBufferMemory
	);

	//copy contents of our hard coded triangle into the staging buffer
	void* mappedMemory;
	vkMapMemory(device, stagingBufferMemory, 0, vertexArraySize, 0, &mappedMemory);
	memcpy(mappedMemory, vertexArray.data(), (size_t)vertexArraySize);
	vkUnmapMemory(device, stagingBufferMemory);

	//copy contents of staging buffer to vertex buffer
	Utils::copyBuffer(stagingBuffer, vertexBuffer, vertexArraySize);

	//destroy staging buffer
	vkDestroyBuffer(device, stagingBuffer, NULL);
	vkFreeMemory(device, stagingBufferMemory, NULL);
}

void RenderApplication::createIndexBuffer(){

	VkDeviceSize indexArraySize = indexArray.size() * sizeof(uint32_t);

	Utils::createBuffer(
		indexArraySize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer, indexBufferMemory
	);
}

void RenderApplication::writeToIndexBuffer(){

	VkDeviceSize indexArraySize = indexArray.size() * sizeof(uint32_t);

	//create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;


	Utils::createBuffer(
		indexArraySize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		stagingBuffer, stagingBufferMemory
	);

	//copy contents of our hard coded triangle indices into the staging buffer
	void* mappedMemory;
	vkMapMemory(device, stagingBufferMemory, 0, indexArraySize, 0, &mappedMemory);
	memcpy(mappedMemory, indexArray.data(), (size_t)indexArraySize);
	vkUnmapMemory(device, stagingBufferMemory);

	//copy contents of staging buffer to index buffer
	Utils::copyBuffer(stagingBuffer, indexBuffer, indexArraySize);

	//destroy staging buffer
	vkDestroyBuffer(device, stagingBuffer, NULL);
	vkFreeMemory(device, stagingBufferMemory, NULL);
}

void RenderApplication::createUniformBuffers(){

	
	tessShaderUBOs.resize(SwapChain::images.size());
	tessShaderUBOMemories.resize(SwapChain::images.size());
	fragShaderUBOs.resize(SwapChain::images.size());
	fragShaderUBOMemories.resize(SwapChain::images.size());

	for(unsigned int i = 0; i < SwapChain::images.size(); ++i){
		//Create UBO for the tess shader data
		Utils::createBuffer(
			sizeof(UniformDataTessShader),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			tessShaderUBOs[i], tessShaderUBOMemories[i]
		);

		//Create UBO for the fragment shader data
		Utils::createBuffer(
			sizeof(UniformDataFragShader),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			fragShaderUBOs[i], fragShaderUBOMemories[i]
		);
	}

}

void RenderApplication::writeToUniformBuffer(uint32_t imageIndex){

	modelRotation += 0.2f;

	//Copy over Vertex Shader UBO
    UniformDataTessShader tessShaderData;

	glm::vec3 cameraPosition(0, 7.8f, 8.8f);
	tessShaderData.model = 
		glm::translate(glm::mat4(1.0f), glm::vec3(0,0.75f,0)) * 
		glm::rotate(glm::mat4(1.0f), glm::radians(modelRotation), glm::vec3(0,1,0)) * 
		glm::scale(glm::mat4(1.0f), glm::vec3(0.03f, 0.03f, 0.03f));
	tessShaderData.view = glm::lookAt(cameraPosition, glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
	tessShaderData.projection = glm::perspective(glm::radians(45.0f), (float)(SwapChain::extent.width) / SwapChain::extent.height, 0.2f, 300.0f);
	tessShaderData.projection[1][1] *= -1;
	
	
	//Copy over Fragment Shader UBO
	UniformDataFragShader fragShaderData;
	fragShaderData.lightDirection = glm::normalize(glm::vec3(2.5f, -2, -3.5f));
	fragShaderData.textureParam = 0.7f;
	fragShaderData.cameraPosition = cameraPosition;
	fragShaderData.matColor = glm::vec3(1, 1, 1);

	void* mappedMemory;

	
	vkMapMemory(device, tessShaderUBOMemories[imageIndex], 0, sizeof(tessShaderData), 0, &mappedMemory);
	memcpy(mappedMemory, &tessShaderData, sizeof(tessShaderData));
	vkUnmapMemory(device, tessShaderUBOMemories[imageIndex]);

	vkMapMemory(device, fragShaderUBOMemories[imageIndex], 0, sizeof(fragShaderData), 0, &mappedMemory);
	memcpy(mappedMemory, &fragShaderData, sizeof(fragShaderData));
	vkUnmapMemory(device, fragShaderUBOMemories[imageIndex]);



}

void RenderApplication::createDiffuseTexture(){

	Utils::createImageFromFile(
		"resources/images/rock.jpg",	//File name on Disk
		diffuseTexture,					//image
		diffuseTextureMemory,			//image memory
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL	//Layout of image
	);

}

void RenderApplication::createDiffuseTextureView(){
	Utils::createImageView(diffuseTexture, diffuseTextureView, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

void RenderApplication::createEnvironmentMap() {

	std::vector<string> faceNames;
	faceNames.push_back("resources/images/ocean view/right.jpg");
	faceNames.push_back("resources/images/ocean view/left.jpg");
	faceNames.push_back("resources/images/ocean view/top.jpg");
	faceNames.push_back("resources/images/ocean view/bottom.jpg");
	faceNames.push_back("resources/images/ocean view/back.jpg");
	faceNames.push_back("resources/images/ocean view/front.jpg");
	Utils::createCubeMapImageFromFile(faceNames, environmentMap, environmentMapMemory, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

}

void RenderApplication::createEnvironmentMapView() {
	Utils::createImageView(environmentMap, environmentMapView, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, true);
}

void RenderApplication::createTextureSampler(){
	Utils::createImageSampler(textureSampler);
}


void RenderApplication::createDepthAttachmentImage(){

	Utils::createImage(
		SwapChain::extent,
		VK_FORMAT_D32_SFLOAT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthAttachmentImage,
		depthAttachmentImageMemory
	);

	//we choose to transition layout here (not in render pass) since the transition only needs to happen once in a realtime app
	Utils::transitionImageLayout(depthAttachmentImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void RenderApplication::createDepthAttachmentImageView(){
	Utils::createImageView(depthAttachmentImage, depthAttachmentImageView, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);
}


void RenderApplication::createSwapChainFrameBuffers() {

	//create swapchain frame buffers
	swapChainFrameBuffers.resize(SwapChain::images.size());

	for (unsigned int i = 0; i < swapChainFrameBuffers.size(); ++i) {

		//define attachments for this frame buffer
		std::array<VkImageView, 2> imageAttachments = {
			SwapChain::imageViews[i],	//color attachment
			depthAttachmentImageView	//depth attachment
		};


		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderPass;
		createInfo.attachmentCount = (uint32_t)imageAttachments.size();
		createInfo.pAttachments = imageAttachments.data();
		createInfo.width = SwapChain::extent.width;
		createInfo.height = SwapChain::extent.height;
		createInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(device, &createInfo, NULL, &swapChainFrameBuffers[i]));
	}
}
void RenderApplication::createDescriptorSetLayout() {

    //define a binding for our tesselation shader UBO
    VkDescriptorSetLayoutBinding vertexUBOBinding = {};
	vertexUBOBinding.binding = 0;	//binding = 0
	vertexUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vertexUBOBinding.descriptorCount = 1;
	vertexUBOBinding.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

	//define a binding for our diffuse texture
	VkDescriptorSetLayoutBinding diffuseTextureBinding = {};
	diffuseTextureBinding.binding = 1;	//binding = 1
	diffuseTextureBinding.descriptorCount = 1;
	diffuseTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	diffuseTextureBinding.pImmutableSamplers = NULL;
	diffuseTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//define a binding for our environment map
	VkDescriptorSetLayoutBinding environmentMapBinding = {};
	environmentMapBinding.binding = 2;	//binding 2
	environmentMapBinding.descriptorCount = 1;
	environmentMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	environmentMapBinding.pImmutableSamplers = NULL;
	environmentMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//define a binding for our fragment shader UBO
	VkDescriptorSetLayoutBinding fragmentUBOBinding = {};
	fragmentUBOBinding.binding = 3;	//binding = 3
	fragmentUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	fragmentUBOBinding.descriptorCount = 1;
	fragmentUBOBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


    //put all bindings in an array
    std::array<VkDescriptorSetLayoutBinding, 4> allBindings = { vertexUBOBinding, diffuseTextureBinding, environmentMapBinding, fragmentUBOBinding };

    //create descriptor set layout for binding to a UBO
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = (uint32_t)allBindings.size(); //number of bindings
    layoutInfo.pBindings = allBindings.data();

    // Create the descriptor set layout.
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout));

}

void RenderApplication::createDescriptorPool(){


    std::array<VkDescriptorPoolSize, 4> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (uint32_t)SwapChain::images.size();
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = (uint32_t)SwapChain::images.size();
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = (uint32_t)SwapChain::images.size();
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[3].descriptorCount = (uint32_t)SwapChain::images.size();

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = (uint32_t)SwapChain::images.size();
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

    //Create descriptor pool.
    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool));


}
void RenderApplication::createDescriptorSets() {


	descriptorSets.resize(SwapChain::images.size());

	//All descriptor sets use same layout
	std::vector<VkDescriptorSetLayout> allLayouts(SwapChain::images.size(), descriptorSetLayout);

    //With the pool allocated, we can now allocate the descriptor set.
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)SwapChain::images.size(); // one descriptor set per swapchain image
    descriptorSetAllocateInfo.pSetLayouts = allLayouts.data();


    // allocate descriptor set.
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, descriptorSets.data()));

	for(unsigned int i = 0; i < SwapChain::images.size(); ++i){

		// Specify the vertex UBO info
		VkDescriptorBufferInfo descriptorVertexUBOInfo = {};
		descriptorVertexUBOInfo.buffer = tessShaderUBOs[i];		//hard coded for now
		descriptorVertexUBOInfo.offset = 0;
		descriptorVertexUBOInfo.range = sizeof(UniformDataTessShader);

		// Specify the diffuse texture info
		VkDescriptorImageInfo descriptorDiffuseTextureInfo = {};
		descriptorDiffuseTextureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorDiffuseTextureInfo.imageView = diffuseTextureView;
		descriptorDiffuseTextureInfo.sampler = textureSampler;

		// Specify the envirmnemt map info
		VkDescriptorImageInfo descriptorEnvironmentMapInfo = {};
		descriptorEnvironmentMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorEnvironmentMapInfo.imageView = environmentMapView;
		descriptorEnvironmentMapInfo.sampler = textureSampler;

		// Specify the fragment UBO info
		VkDescriptorBufferInfo descriptorFragmentUBOInfo = {};
		descriptorFragmentUBOInfo.buffer = fragShaderUBOs[i]; 	//hard coded for now
		descriptorFragmentUBOInfo.offset = 0;
		descriptorFragmentUBOInfo.range = sizeof(UniformDataFragShader);


		std::array<VkWriteDescriptorSet, 4> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;		//binding = 0
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &descriptorVertexUBOInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;		//binding = 1
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &descriptorDiffuseTextureInfo;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;		//binding = 2
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &descriptorEnvironmentMapInfo;

		descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[3].dstSet = descriptorSets[i];
		descriptorWrites[3].dstBinding = 3;		//binding = 3
		descriptorWrites[3].dstArrayElement = 0;
		descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[3].descriptorCount = 1;
		descriptorWrites[3].pBufferInfo = &descriptorFragmentUBOInfo;

		// perform the update of the descriptor set.
		vkUpdateDescriptorSets(device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, NULL);

	}

}


void RenderApplication::createCommandPool(){

    //We are getting closer to the end. In order to send commands to the device(GPU),
    //we must first record commands into a command buffer.
    //To allocate a command buffer, we must first create a command pool. So let us do that.
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = 0;

    // the queue family of this command pool. All command buffers allocated from this command pool,
    // must be submitted to queues of this family ONLY.
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.getQueueFamilyIndexAt(0);	//graphics QFI
    VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &graphicsCommandPool));
}


void RenderApplication::createRenderPass() {

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		//we don't care what the inital layout of the image is
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	//want to present the swapchain images after we render to them

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (uint32_t)attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}
void RenderApplication::createGraphicsPipeline(){

	
	//Vertex Shader Stage
	VkShaderModule vertexShaderModule = Utils::createShaderModule("resources/shaders/vert.spv");

	VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertexShaderModule;
	vertexShaderStageInfo.pName = "main";

	//Tessalation Control Shader Stage
	VkShaderModule tessContShaderModule = Utils::createShaderModule("resources/shaders/tesc.spv");

	VkPipelineShaderStageCreateInfo tessContShaderStageInfo = {};
	tessContShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	tessContShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	tessContShaderStageInfo.module = tessContShaderModule;
	tessContShaderStageInfo.pName = "main";

	//Tessalation Control Shader Stage
	VkShaderModule tessEvalShaderModule = Utils::createShaderModule("resources/shaders/tese.spv");

	VkPipelineShaderStageCreateInfo tessEvalShaderStageInfo = {};
	tessEvalShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	tessEvalShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	tessEvalShaderStageInfo.module = tessEvalShaderModule;
	tessEvalShaderStageInfo.pName = "main";
	
	//Fragment Shader Stage
	VkShaderModule fragmentShaderModule = Utils::createShaderModule("resources/shaders/frag.spv");

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragmentShaderModule;
	fragmentShaderStageInfo.pName = "main";

	
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, tessContShaderStageInfo, 
	tessEvalShaderStageInfo, fragmentShaderStageInfo };

    //Vertex Input
	auto vertexBindingDescription = Vertex::getBindingDescription();
	auto vertexAttributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertexAttributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();


    //Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

	//Tesselation
	VkPipelineTessellationStateCreateInfo tessellation = {};
	tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tessellation.pNext = NULL;
	tessellation.flags = 0;
	tessellation.patchControlPoints = 3;

    //Viewports
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0;
	viewport.width = (float)SwapChain::extent.width;
    viewport.height = (float)SwapChain::extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Scissors
    VkRect2D scissor = {};
    scissor.offset = {0,0};
    scissor.extent = SwapChain::extent;

    //Viewport State
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    //Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	//Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//Depth Testing
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;


	//Color Blending (per frame buffer)
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;


	//Color blending (global settings)
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	//Pipeline Layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;


	//Create Pipeline Layout
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout));


	//Info to create graphics pipeline, Note: we can create more for multiple pipelines (shadow map + standard render)
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 4;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pTessellationState = &tessellation;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = NULL;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	//Create Graphics Pipeline 
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline));


	//destroy shader modules since we don't need their source code anymore
	vkDestroyShaderModule(device,vertexShaderModule, NULL);
	vkDestroyShaderModule(device, fragmentShaderModule, NULL);
	vkDestroyShaderModule(device, tessContShaderModule, NULL);
	vkDestroyShaderModule(device, tessEvalShaderModule, NULL);

}


void RenderApplication::createSyncObjects(){

	//Each frame in flight will have each of these sync objects
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i){
		VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &inFlightFences[i]));
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &imageAvailableSemaphores[i]));
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &renderFinishedSemaphores[i]));
	}


}
void RenderApplication::createRenderCommandBuffers() {

	//we want each of our command buffers to draw to a swapchain image
	renderCommandBuffers.resize(SwapChain::images.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)renderCommandBuffers.size();

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, renderCommandBuffers.data()));

	for (unsigned int i = 0; i < renderCommandBuffers.size(); ++i) {

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = NULL;

		//main command buffer scope
		VK_CHECK_RESULT(vkBeginCommandBuffer(renderCommandBuffers[i], &beginInfo));

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFrameBuffers[i];
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = SwapChain::extent;

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			clearValues[1].depthStencil = { 1, 0 };

			renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
			renderPassInfo.pClearValues = clearValues.data();

			//render pass scope
			vkCmdBeginRenderPass(renderCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				//bind our graphics pipeline
				vkCmdBindPipeline(renderCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

				//bind vertex buffer
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(renderCommandBuffers[i], 0, 1, &vertexBuffer, offsets);

				//bind index buffer
				vkCmdBindIndexBuffer(renderCommandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

				//bind descriptor set
				vkCmdBindDescriptorSets(renderCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, NULL);

				//invoke graphics pipeline and draw
				vkCmdDrawIndexed(renderCommandBuffers[i], (uint32_t)indexArray.size(), 1, 0, 0, 0);

			vkCmdEndRenderPass(renderCommandBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(renderCommandBuffers[i]));
	}
	
}


VkCommandPool& RenderApplication::getTransferCmdPool(){

	//queues and pools which work for graphics also work for transfer
	return graphicsCommandPool;
}
VkQueue& RenderApplication::getTransferQueue(){
	return transferQueue;
}
