#include <RenderApplication.h>

#define MAX_FRAMES_IN_FLIGHT 2

//Initialize static members
GLFWwindow* RenderApplication::window;
bool RenderApplication::windowResized = false;
VkExtent2D RenderApplication::desiredInitialExtent = {1920,1470};
std::vector<const char*> RenderApplication::requiredInstanceLayers;
std::vector<const char*> RenderApplication::requiredInstanceExtensions;
std::vector<const char*> RenderApplication::requiredDeviceExtensions;
VkPhysicalDeviceFeatures RenderApplication::requiredDeviceFeatures = {};


VkInstance RenderApplication::instance;
VkDebugReportCallbackEXT RenderApplication::debugReportCallback;
VkPhysicalDevice RenderApplication::physicalDevice;
VkDevice RenderApplication::device;
VkQueue RenderApplication::graphicsQueue;
VkQueue RenderApplication::transferQueue;
VkQueue RenderApplication::presentQueue;
VkQueue RenderApplication::computeQueue;
VkSurfaceKHR RenderApplication:: surface;
std::vector<VkBuffer> RenderApplication::vertexShaderUBOs;
std::vector<VkDeviceMemory> RenderApplication::vertexShaderUBOMemories;
std::vector<VkBuffer> RenderApplication::fragShaderUBOs;
std::vector<VkDeviceMemory> RenderApplication::fragShaderUBOMemories;
VkSampler RenderApplication::imageSampler;
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
std::vector<VkSemaphore> RenderApplication::startPhysicsSemaphores;
int RenderApplication::currentFrame;
VkCommandPool RenderApplication::graphicsCommandPool;
std::vector<VkCommandBuffer> RenderApplication::renderCommandBuffers;

float RenderApplication::currTime;
float RenderApplication::prevTime;
float RenderApplication::deltaTime;
glm::vec2 RenderApplication::mousePosition;
glm::vec2 RenderApplication::prevMousePosition;
bool RenderApplication::isLeftMouseButtonDown = false;
bool RenderApplication::isRightMouseButtonDown = false;
float RenderApplication::mouseWheelDelta = 0;
glm::mat4 RenderApplication::modelCorrect = glm::scale(glm::mat4(1.0f), glm::vec3(0.03f,0.03f,0.03f));
glm::mat4 RenderApplication::cameraHeading = glm::mat4(1.0f);
glm::mat4 RenderApplication::cameraPitch = glm::mat4(1.0f);
glm::mat4 RenderApplication::cameraZoom = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 12));
glm::mat3 RenderApplication::lightOrientation(1.0f);

void RenderApplication::run() {

	//init glfw window
	initGLFWWindow();

	//create all vulkan resources we will need in the app
	createAllVulkanResources();

	//initialize scene variables 
	initScene();
	
	
	cout << "In Main Loop" << endl;
	currentFrame = 0;	//set beginning frame to work with
	
	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
		drawFrame();
		updateScene();
	}

	// wait for remaining images to finish rendering and presenting
	vkDeviceWaitIdle(device);

    // Clean up all resources.
    cleanup();

	cout << "Successful Close" << endl;
}

void RenderApplication::initGLFWWindow(){

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(desiredInitialExtent.width,desiredInitialExtent.height, "Test Window", NULL, NULL);

	//setup callbacks
	glfwSetFramebufferSizeCallback(window, &windowResizeCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorMovedCallback);
	glfwSetScrollCallback(window, mouseWheelCallback);
}

//this function is called repetitively until the user stops resizing the window
void RenderApplication::windowResizeCallback(GLFWwindow* resizedWindow, int newWidth, int newHeight){

	windowResized = true;
}

void RenderApplication::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	
	if (action == GLFW_PRESS) {

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			isLeftMouseButtonDown = true;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			isRightMouseButtonDown = true;
		}
	}

	if (action == GLFW_RELEASE) {

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			isLeftMouseButtonDown = false;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			isRightMouseButtonDown = false;
		}
	}
	
}
void RenderApplication::cursorMovedCallback(GLFWwindow* window, double xpos, double ypos) {
	mousePosition = glm::vec2(xpos, ypos);

}

void RenderApplication::mouseWheelCallback(GLFWwindow* window, double xpos, double ypos) {

	mouseWheelDelta = (float)ypos;
}
VkExtent2D RenderApplication::waitToGetNonZeroWindowExtent() {

	int width = 0, height = 0;

	// we don't want to create the swapchain with width/height values of 0
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	VkExtent2D actualWindowExtent = {
		(uint32_t)width,
		(uint32_t)height
	};

	return actualWindowExtent;
}

int RenderApplication::getPrevFrameIndex(int currFrameIndex){
	if(currFrameIndex == 0){
		return MAX_FRAMES_IN_FLIGHT - 1;
	}
	return currFrameIndex - 1;
}

void RenderApplication::createAllVulkanResources() {

	//add all requirements that this app will need from the device and instance
	configureAllRequirements();

	// Initialize vulkan
	createInstance();
	createSurface();
	findPhysicalDevice();
	createDevice();

	//create swapchain with valid extent
	VkExtent2D initialExtent = waitToGetNonZeroWindowExtent();
	createSwapChain(initialExtent);

	//create descriptor and command resources
	createDescriptorSetLayout();
	createDescriptorPool();
	createCommandPool();

	//GPU resources
	ParticleSystem::init();

	createUniformBuffers();

	createImageSampler();

	createDepthAttachmentImage();
	createDepthAttachmentImageView();
	
	//create descriptors
	createDescriptorSets();

	//create rendering utils
	createRenderPass();	
	createSwapChainFrameBuffers();
	createGraphicsPipeline();
	createSyncObjects();

	//record command buffer
	createRenderCommandBuffers();

	
}

void RenderApplication::initScene(){
	
	//time
	currTime = (float)glfwGetTime();
	prevTime = (float)glfwGetTime();
	deltaTime = 0;

	//mouse
	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	mousePosition = glm::vec2(xPos, yPos);
	prevMousePosition = glm::vec2(xPos, yPos);
}

void RenderApplication::drawFrame(){


	uint64_t MAX_UNSIGNED_64_BIT_VAL = std::numeric_limits<uint64_t>::max();

	VkResult result;
	uint32_t imageIndex;

	//wait for image at current frame to finish rendering
	vkWaitForFences(device,1, &inFlightFences[currentFrame], VK_TRUE, MAX_UNSIGNED_64_BIT_VAL);


	//acquire next image from swapchain, which may currently still be presenting
	result = vkAcquireNextImageKHR(
		device,		//device
		SwapChain::vulkanHandle,	//Vulkan swapchain handle 
		MAX_UNSIGNED_64_BIT_VAL,	//max time to wait to acquire next image
		imageAvailableSemaphores[currentFrame],	//semaphore to signal when image is done presenting previous frame so we can start rendering current frame
		VK_NULL_HANDLE,		//fence to signal when image is done presenting
		&imageIndex			//index of image we are acquiring 
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateAppExtentDependents();
	}
	else {
		VK_CHECK_RESULT(result);
	}


	//update uniform data for this frame
	writeToUniformBuffers(imageIndex);

	//Enqueue render to this swapchain image
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitToRenderSems[] = { imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitToRenderSems;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &renderCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	//SUBMIT A RENDER COMMAND TO THIS IMAGE
	VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]));

	//Enqueue presenting this image
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &SwapChain::vulkanHandle;
	presentInfo.pImageIndices = &imageIndex;

	//SUBMIT A PRESENT COMMAND FOR THIS IMAGE
	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowResized) {
		windowResized = false;
		recreateAppExtentDependents();
	}
	else {
		VK_CHECK_RESULT(result);
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RenderApplication::updateScene() {

	//limit frame rate
	do {
		currTime = (float)glfwGetTime();
	} while (currTime - prevTime < (1.0f / MAX_FRAME_RATE));

	//update deltaTime
	deltaTime = currTime - prevTime;
	prevTime = currTime;

	updateLightRotation();
	updateCameraMatrix();

	mouseWheelDelta = 0;
	prevMousePosition = mousePosition;
}


void RenderApplication::updateLightRotation() {

	glm::vec3 lightSpinAxis;
	float lightSpinAngle;
	if (isRightMouseButtonDown) {
	
		Utils::calcTrackBallDeltas(mousePosition, prevMousePosition, SwapChain::extent, lightSpinAxis, lightSpinAngle);
		glm::mat4 deltaLightRotate = glm::rotate(glm::mat4(1.0f), lightSpinAngle, lightSpinAxis);
		lightOrientation = glm::mat3(deltaLightRotate) * lightOrientation;
	}

}

void RenderApplication::updateCameraMatrix() {

	if (isLeftMouseButtonDown) {
		glm::mat4 rotDeltaX = glm::rotate(glm::mat4(1.0f), 0.003f * (prevMousePosition.x - mousePosition.x), glm::vec3(0, 1, 0));
		cameraHeading = rotDeltaX * cameraHeading;

		glm::mat4 rotDeltaY = glm::rotate(glm::mat4(1.0f), 0.003f * (prevMousePosition.y - mousePosition.y), glm::vec3(1, 0, 0));
		cameraPitch = rotDeltaY * cameraPitch;
	}
	
	cameraZoom = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -mouseWheelDelta * 0.2f)) * cameraZoom;
	
	
}

void RenderApplication::cleanup() {

	//clean up all Window/Vulkan/Scene resources

	ParticleSystem::cleanUp();

	//destroy validation layer callback
	if (enableValidationLayers) {
		
		//load function to clean up validation layer callback
		auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (vkDestroyDebugReportCallbackEXT == nullptr) {
			throw std::runtime_error("Error: Could not load vkDestroyDebugReportCallbackEXT");
		}
		vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, NULL);
	}


	//free uniform buffers
	for(unsigned int i = 0; i < SwapChain::images.size(); ++i){
		vkDestroyBuffer(device, vertexShaderUBOs[i], NULL);
		vkFreeMemory(device, vertexShaderUBOMemories[i], NULL);
		vkDestroyBuffer(device, fragShaderUBOs[i], NULL);
		vkFreeMemory(device, fragShaderUBOMemories[i], NULL);
	}


	//free sampler
	vkDestroySampler(device, imageSampler, NULL);


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
		vkDestroySemaphore(device, startPhysicsSemaphores[i], NULL);
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

	//add glfw instance extensions (after you init glfw window)
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for(unsigned int i = 0; i < glfwExtensionCount; ++i){
		requiredInstanceExtensions.push_back(glfwExtensions[i]);
	}


	//specify required device features 
	requiredDeviceFeatures.samplerAnisotropy = VK_TRUE;
	requiredDeviceFeatures.fillModeNonSolid = VK_TRUE;
	requiredDeviceFeatures.wideLines = VK_TRUE;

	//specify what queue capabilities we need
	QueueFamilyMap::addRequiredQueueType(VK_QUEUE_GRAPHICS_BIT);
	QueueFamilyMap::addRequiredQueueType(VK_QUEUE_TRANSFER_BIT);
	QueueFamilyMap::addRequiredQueueType(ADDITIONAL_VK_QUEUE_PRESENT_BIT);
	QueueFamilyMap::addRequiredQueueType(VK_QUEUE_COMPUTE_BIT);

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


	//check for presence of all required instance extensions
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
        VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
        debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debugCallbackCreateInfo.pfnCallback = &debugReportCallbackFunction;

        // We have to explicitly load this function and have our local function pointer point to it
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        if (vkCreateDebugReportCallbackEXT == nullptr) {
            throw std::runtime_error("Could not load vkCreateDebugReportCallbackEXT");
        }

        // Create and register callback.
        VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(instance, &debugCallbackCreateInfo, NULL, &debugReportCallback));
    }

}

void RenderApplication::createSurface(){

	VK_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));
}
void RenderApplication::findPhysicalDevice() {

    //In this function, we find a physical device that can be used with Vulkan.
    //So, first we will list all physical devices on the system with vkEnumeratePhysicalDevices

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
	// that this physical device supports the swapchain device extension VK_KHR_SWAPCHAIN_EXTENSION_NAME
	SwapChain::querySupportDetails(potentialPhysicalDevice, surface);
	if (!SwapChain::hasAdequateSupport()) {
		return false;
	}

	// Does this physical device have queue family indices for all our required queue types
	QueueFamilyMap::compute(potentialPhysicalDevice, surface);
	if (!QueueFamilyMap::allHaveValues()) {
		return false;
	}

	// all requirements satisfied at this point, this physical device suits our needs
	return true;

}

void RenderApplication::createDevice() {

	//only need unique queue families
	std::set<uint32_t> uniqueQueueFamilies;
	for (int i = 0; i < QueueFamilyMap::numRequired(); ++i) {
		uniqueQueueFamilies.insert(QueueFamilyMap::getQueueFamilyIndexAt(i));
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

	// use the physical device we chose to create logical device.
    VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device)); 

	//the index within this queue family of the queue to retrieve, we just take the first one
    uint32_t particularQueueIndex = 0;

    // Get a handle to the first member of each queue family.
    vkGetDeviceQueue(device, QueueFamilyMap::getQueueFamilyIndexAt(0), particularQueueIndex, &graphicsQueue);
	vkGetDeviceQueue(device, QueueFamilyMap::getQueueFamilyIndexAt(1), particularQueueIndex, &transferQueue);
	vkGetDeviceQueue(device, QueueFamilyMap::getQueueFamilyIndexAt(2), particularQueueIndex, &presentQueue);
	vkGetDeviceQueue(device, QueueFamilyMap::getQueueFamilyIndexAt(3), particularQueueIndex, &computeQueue);
}

void RenderApplication::createSwapChain(const VkExtent2D appExtent) {

	SwapChain::init(
		device,			//App logical device
		surface,		//Vulkan Surface object
		QueueFamilyMap::getQueueFamilyIndexAt(0),	//graphics queue family index
		QueueFamilyMap::getQueueFamilyIndexAt(1),	//present queue family index
		appExtent								//actual extent of the window
	);

}

void RenderApplication::recreateAppExtentDependents(){

	//wait for frames to finish rendering/presenting
	vkDeviceWaitIdle(device);

	
	//free command buffers (while keeping command pool)
	vkFreeCommandBuffers(device, graphicsCommandPool, (uint32_t)SwapChain::images.size(), renderCommandBuffers.data());

	//destroy framebuffers
	for(unsigned int i = 0; i < SwapChain::images.size(); ++i){
		vkDestroyFramebuffer(device, swapChainFrameBuffers[i], NULL);
	}

	//destroy depth image objects
	vkDestroyImageView(device, depthAttachmentImageView, NULL);
	vkDestroyImage(device, depthAttachmentImage, NULL);
	vkFreeMemory(device, depthAttachmentImageMemory, NULL);

	//destroy graphics pipeline
	vkDestroyPipeline(device, graphicsPipeline, NULL);
	vkDestroyPipelineLayout(device, pipelineLayout, NULL);

	//destroy render pass
	vkDestroyRenderPass(device, renderPass, NULL);

	//destroy swapchain
	SwapChain::cleanUp(device);

	
	//we only want to recreate the following vulkan objects with a non zero app extent (width & height)
	VkExtent2D newExtent = waitToGetNonZeroWindowExtent();

	//Recreate previously destroyed objects
	SwapChain::querySupportDetails(physicalDevice, surface);
	createSwapChain(newExtent);
	createRenderPass();
	createGraphicsPipeline();
	createDepthAttachmentImage();
	createDepthAttachmentImageView();
	createSwapChainFrameBuffers();
	createRenderCommandBuffers();
	
}


void RenderApplication::createUniformBuffers(){

	
	vertexShaderUBOs.resize(SwapChain::images.size());
	vertexShaderUBOMemories.resize(SwapChain::images.size());
	fragShaderUBOs.resize(SwapChain::images.size());
	fragShaderUBOMemories.resize(SwapChain::images.size());

	for(unsigned int i = 0; i < SwapChain::images.size(); ++i){
		//Create UBO for the vertex shader data
		Utils::createBuffer(
			sizeof(UniformDataVertexShader),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			vertexShaderUBOs[i], vertexShaderUBOMemories[i]
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

void RenderApplication::writeToUniformBuffers(uint32_t imageIndex){

	
	glm::mat4 cameraMatrix = cameraHeading * cameraPitch * cameraZoom;

	//Copy over Vertex Shader UBO
	UniformDataVertexShader vertexShaderData;
	vertexShaderData.model = modelCorrect;
	vertexShaderData.view = glm::inverse(cameraMatrix);
	vertexShaderData.projection = glm::perspective(glm::radians(45.0f), (float)(SwapChain::extent.width) / SwapChain::extent.height, 0.2f, 300.0f);
	vertexShaderData.projection[1][1] *= -1;
	
	
	//Copy over Fragment Shader UBO
	UniformDataFragShader fragShaderData;
	fragShaderData.lightDirection = lightOrientation * Lighting::direction;
	fragShaderData.textureParam = 0.65f;
	fragShaderData.cameraPosition = cameraMatrix * glm::vec4(0,0,0,1);
	fragShaderData.normalMapStrength = 0.7f;
	fragShaderData.matColor = glm::vec3(1, 1, 1);
	
	void* mappedMemory;

	
	vkMapMemory(device, vertexShaderUBOMemories[imageIndex], 0, sizeof(vertexShaderData), 0, &mappedMemory);
	memcpy(mappedMemory, &vertexShaderData, sizeof(vertexShaderData));
	vkUnmapMemory(device, vertexShaderUBOMemories[imageIndex]);

	vkMapMemory(device, fragShaderUBOMemories[imageIndex], 0, sizeof(fragShaderData), 0, &mappedMemory);
	memcpy(mappedMemory, &fragShaderData, sizeof(fragShaderData));
	vkUnmapMemory(device, fragShaderUBOMemories[imageIndex]);

}

void RenderApplication::createImageSampler(){
	Utils::createImageSampler(imageSampler);
}


void RenderApplication::createDepthAttachmentImage(){

	Utils::createImage(
		SwapChain::extent,
		VK_FORMAT_D32_SFLOAT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
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
	
    //define a binding for our vertex shader UBO
    VkDescriptorSetLayoutBinding vertexShaderUBOBinding = {};
	vertexShaderUBOBinding.binding = 0;	//binding = 0
	vertexShaderUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vertexShaderUBOBinding.descriptorCount = 1;
	vertexShaderUBOBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


	//define a binding for our fragment shader UBO
	VkDescriptorSetLayoutBinding fragShaderUBOBinding = {};
	fragShaderUBOBinding.binding = 1;	//binding = 1
	fragShaderUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	fragShaderUBOBinding.descriptorCount = 1;
	fragShaderUBOBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


    //put all bindings in an array
    std::array<VkDescriptorSetLayoutBinding, 2> allBindings = { 
		vertexShaderUBOBinding, 
		fragShaderUBOBinding	
	};

    //create descriptor set layout for all above binding points
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = (uint32_t)allBindings.size(); //number of bindings
    layoutInfo.pBindings = allBindings.data();

    // Create the descriptor set layout.
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout));

}

void RenderApplication::createDescriptorPool(){


    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (uint32_t)SwapChain::images.size();
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = (uint32_t)SwapChain::images.size();
	

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

	//All descriptor sets use same layout, we need one descriptor set per swapchain image
	std::vector<VkDescriptorSetLayout> allLayouts(SwapChain::images.size(), descriptorSetLayout);

    //With the pool allocated, we can now allocate the descriptor set.
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)descriptorSets.size(); // one descriptor set per swapchain image
    descriptorSetAllocateInfo.pSetLayouts = allLayouts.data();


    // allocate descriptor set.
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, descriptorSets.data()));

	for(unsigned int i = 0; i < SwapChain::images.size(); ++i){

		// Descriptor for our vertex shader Uniform Buffer
		VkWriteDescriptorSet vertexUBODescriptorWrite = {};
		vertexUBODescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vertexUBODescriptorWrite.dstSet = descriptorSets[i];
		vertexUBODescriptorWrite.dstBinding = 0;		//binding = 0
		vertexUBODescriptorWrite.dstArrayElement = 0;	
			// Descriptor info
			VkDescriptorBufferInfo vertexUBODescriptorInfo = {};
			vertexUBODescriptorInfo.buffer = vertexShaderUBOs[i];		
			vertexUBODescriptorInfo.offset = 0;
			vertexUBODescriptorInfo.range = sizeof(UniformDataVertexShader);

		vertexUBODescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vertexUBODescriptorWrite.descriptorCount = 1;
		vertexUBODescriptorWrite.pBufferInfo = &vertexUBODescriptorInfo;


		// Descriptor for our fragment shader Uniform Buffer
		VkWriteDescriptorSet fragmentUBODescriptorWrite = {};
		fragmentUBODescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		fragmentUBODescriptorWrite.dstSet = descriptorSets[i];
		fragmentUBODescriptorWrite.dstBinding = 1;		//binding = 1
		fragmentUBODescriptorWrite.dstArrayElement = 0;
			// Descriptor info
			VkDescriptorBufferInfo fragmentUBODescriptorInfo = {};
			fragmentUBODescriptorInfo.buffer = fragShaderUBOs[i];
			fragmentUBODescriptorInfo.offset = 0;
			fragmentUBODescriptorInfo.range = sizeof(UniformDataFragShader);

		fragmentUBODescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		fragmentUBODescriptorWrite.descriptorCount = 1;
		fragmentUBODescriptorWrite.pBufferInfo = &fragmentUBODescriptorInfo;


		//put all descriptor write info in an array
		std::array<VkWriteDescriptorSet, 2> descriptorWrites = { 
			vertexUBODescriptorWrite,
			fragmentUBODescriptorWrite
		};

		// perform the update of the descriptor sets
		vkUpdateDescriptorSets(device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, NULL);

	}//End for each swap chain image

}


void RenderApplication::createCommandPool(){

    //In order to send commands to the device(GPU),
    //we must first record commands into a command buffer.
    //Command buffers are allocated from command pools, which manage their memory
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = 0;

    // the queue family of this command pool. All command buffers allocated from this command pool,
    // must be submitted to queues of this family ONLY.
    commandPoolCreateInfo.queueFamilyIndex = QueueFamilyMap::getQueueFamilyIndexAt(0);	//graphics QFI
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
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;   //Note: use VK_ATTACHMENT_STORE_OP_STORE for shadow maps
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

	
	//Fragment Shader Stage
	VkShaderModule fragmentShaderModule = Utils::createShaderModule("resources/shaders/frag.spv");

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragmentShaderModule;
	fragmentShaderStageInfo.pName = "main";

	
	VkPipelineShaderStageCreateInfo shaderStages[] = { 
		vertexShaderStageInfo, 
		fragmentShaderStageInfo 
	};

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
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;


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

	//Pipeline Layout, need to know about descriptor set layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;


	//Create Pipeline Layout
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout));


	//Info to create graphics pipeline, Note: we can create more for multiple pipelines (shadow map + standard render)
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;	//can't be non-null if using color attachments
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

}


void RenderApplication::createSyncObjects(){

	//Each frame in flight will have each of these sync objects
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	startPhysicsSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	//Syncronize rendering with presenting on GPU
	//Note: All semaphores are created in the unsignaled state
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i){
		VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &inFlightFences[i]));
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &imageAvailableSemaphores[i]));
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &renderFinishedSemaphores[i]));
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &startPhysicsSemaphores[i]));
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
				vkCmdBindVertexBuffers(renderCommandBuffers[i], 0, 1, &ParticleSystem::vertexBuffer, offsets);
				
				//bind descriptor set
				vkCmdBindDescriptorSets(renderCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, NULL);

				//invoke graphics pipeline and draw
				vkCmdDraw(renderCommandBuffers[i], (uint32_t)ParticleSystem::particleArray.size(), 1, 0, 0);

			vkCmdEndRenderPass(renderCommandBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(renderCommandBuffers[i]));
	}
	
}

VkCommandPool RenderApplication::getTransferCmdPool(){

	//queues and pools which work for graphics also work for transfer
	return graphicsCommandPool;
}
VkQueue RenderApplication::getTransferQueue(){

	return transferQueue;
}

VkCommandPool RenderApplication::getGraphicsCmdPool() {
	return graphicsCommandPool;
}

VkQueue RenderApplication::getGraphicsQueue(){
	return graphicsQueue;
}

VkCommandPool RenderApplication::getComputeCommandPool() {

	//queues and pools which work for graphics also work for compute
	return graphicsCommandPool;
}
VkQueue RenderApplication::getComputeQueue() {
	return computeQueue;
}