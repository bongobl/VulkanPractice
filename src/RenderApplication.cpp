#include <RenderApplication.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


//Initialize static members
VkExtent2D RenderApplication::resolution = {1280,980};
std::vector<const char*> RenderApplication::requiredInstanceLayers;
std::vector<const char*> RenderApplication::requiredInstanceExtensions;
std::vector<const char*> RenderApplication::requiredDeviceExtensions;
VkPhysicalDeviceFeatures RenderApplication::requiredDeviceFeatures = {};
std::vector<VkQueueFlags> RenderApplication::requiredQueueTypes;

VkInstance RenderApplication::instance;
VkDebugReportCallbackEXT RenderApplication::debugReportCallback;
VkPhysicalDevice RenderApplication::physicalDevice;
VkDevice RenderApplication::device;
std::vector<Vertex> RenderApplication::vertexArray;
std::vector<uint32_t> RenderApplication::indexArray;
VkBuffer RenderApplication::vertexBuffer;
VkDeviceMemory RenderApplication::vertexBufferMemory;
VkBuffer RenderApplication::indexBuffer;
VkDeviceMemory RenderApplication::indexBufferMemory;
VkBuffer RenderApplication::uniformBuffer;
VkDeviceMemory RenderApplication::uniformBufferMemory;
VkImage RenderApplication::diffuseTexture;
VkDeviceMemory RenderApplication::diffuseTextureMemory;
VkImageView RenderApplication::diffuseTextureView;
VkImage RenderApplication::environmentMap;
VkDeviceMemory RenderApplication::environmentMapMemory;
VkImageView RenderApplication::environmentMapView;
VkSampler RenderApplication::textureSampler;
VkImage RenderApplication::colorAttachmentImage;
VkDeviceMemory RenderApplication::colorAttachmentImageMemory;
VkImageView RenderApplication::colorAttachmentImageView;
VkImage RenderApplication::depthAttachmentImage;
VkDeviceMemory RenderApplication::depthAttachmentImageMemory;
VkImageView RenderApplication::depthAttachmentImageView;
VkFramebuffer RenderApplication::frameBuffer;
VkDescriptorPool RenderApplication::descriptorPool;
VkDescriptorSet RenderApplication::descriptorSet;
VkDescriptorSetLayout RenderApplication::descriptorSetLayout;
VkRenderPass RenderApplication::renderPass;
VkPipelineLayout RenderApplication::pipelineLayout;
VkPipeline RenderApplication::graphicsPipeline;
VkCommandPool RenderApplication::graphicsCommandPool;
VkCommandBuffer RenderApplication::mainCommandBuffer;
uint32_t RenderApplication::graphicsQueueFamilyIndex;
VkQueue RenderApplication::graphicsQueue;


void RenderApplication::run() {

	//add all requirements that this app will need from the device and instance
	configureAllRequirements();

    // Initialize vulkan
    createInstance();
    findPhysicalDevice();
    createDevice();


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

    createUniformBuffer();
    writeToUniformBuffer();

	createDiffuseTexture();
	createDiffuseTextureView();

	createEnvironmentMap();
	createEnvironmentMapView();

	createTextureSampler();

	createColorAttachmentImage();
	createColorAttachmentImageView();

	createDepthAttachmentImage();
	createDepthAttachmentImageView();


	//create descriptors
    createDescriptorSet();

    //create rendering utils
	createRenderPass();
	createFrameBuffer();
    createGraphicsPipeline();

    //record command buffer
    createMainCommandBuffer();

    cout << "Rendering Scene" << endl;

    // Finally, run the recorded command buffer.
    runMainCommandBuffer();

    cout << "Exporting Image to Disk" << endl;
    //export the contents of the color attachment to disk
	exportAsImage();

    // Clean up all Vulkan resources.
    cleanup();
}

void RenderApplication::configureAllRequirements(){


	if(enableValidationLayers){
		cout << "<Debug Mode: Validation Layers Enabled>" << endl;

		//our validation requires one instance layer and one instance extension
		requiredInstanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		requiredInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	//Note: In realtime app, glfw will have extra instance extensions we need to add.
	//And usage of the swapchain would require a device extension. 

	//specify required device features 
	requiredDeviceFeatures.samplerAnisotropy = VK_TRUE;
	requiredDeviceFeatures.tessellationShader = VK_TRUE;

	//specify what capabilities we need from a queue	
	requiredQueueTypes.push_back(VK_QUEUE_GRAPHICS_BIT);
	requiredQueueTypes.push_back(VK_QUEUE_TRANSFER_BIT); 
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
			string errorMessage = "Layer " + string(currRequiredLayer) + " not supported\n";
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
			string errorMessage = "Extension " + string(currRequiredExtension) + " not supported\n";
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

		if (isValidPhysicalDevice(currPhysicalDevice, graphicsQueueFamilyIndex)) {
			physicalDevice = currPhysicalDevice;
			return;
		}
    }

	throw std::runtime_error("Error: Could not load find a valid physical device for our operations");
}

bool RenderApplication::isValidPhysicalDevice(VkPhysicalDevice potentialPhysicalDevice, uint32_t &familyIndex) {

	//For this physical device to be valid, it needs to support our device features, device extensions and have
	//a queue family for our operations

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


	//lastly, we make sure there exists a queue family index that supports the operations we want
	familyIndex = getQueueFamilyIndex(potentialPhysicalDevice);
	return familyIndex != -1;


}
// Returns the index of a queue family that supports compute and graphics operations.
uint32_t RenderApplication::getQueueFamilyIndex(VkPhysicalDevice currPhysicalDevice) {

    uint32_t queueFamilyCount;

	// Retrieve all queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(currPhysicalDevice, &queueFamilyCount, NULL);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(currPhysicalDevice, &queueFamilyCount, queueFamilies.data());

    // Now find a family that supports graphics and transfer.
    uint32_t currFamilyIndex;
    for (currFamilyIndex = 0; currFamilyIndex < queueFamilies.size(); ++currFamilyIndex) {
        VkQueueFamilyProperties currFamily = queueFamilies[currFamilyIndex];


		if (currFamily.queueCount > 0) {

			bool hasAllQueueTypes = true;
			for (int i = 0; i < requiredQueueTypes.size(); ++i) {
				if (! (currFamily.queueFlags & requiredQueueTypes[i])) {
					hasAllQueueTypes = false;
					break;
				}
			}
			if (hasAllQueueTypes) {

				// found a queue family with out required queue types, we're done
				break;
			}
		}
    }

    if (currFamilyIndex == queueFamilies.size()) {
		return -1;
    }
	
    return currFamilyIndex;
}

void RenderApplication::createDevice() {


    //When creating the device, we also specify what queues it has.
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount = 1; // get one queue from this family. We don't need more.
    float queuePriorities = 1.0;  // we only have one queue, so this is not that imporant.
    queueCreateInfo.pQueuePriorities = &queuePriorities;


    //Now we create the logical device. The logical device allows us to interact with the physical device.
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; // when creating the logical device, we also specify what queues it has.
    deviceCreateInfo.queueCreateInfoCount = 1;
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
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, particularQueueIndex, &graphicsQueue);
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
	memcpy(mappedMemory, vertexArray.data(), vertexArraySize);
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
	memcpy(mappedMemory, indexArray.data(), indexArraySize);
	vkUnmapMemory(device, stagingBufferMemory);

	//copy contents of staging buffer to index buffer
	Utils::copyBuffer(stagingBuffer, indexBuffer, indexArraySize);

	//destroy staging buffer
	vkDestroyBuffer(device, stagingBuffer, NULL);
	vkFreeMemory(device, stagingBufferMemory, NULL);
}

void RenderApplication::createUniformBuffer(){

	Utils::createBuffer(
		sizeof(UniformBufferObject),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		uniformBuffer, uniformBufferMemory
	);

}

void RenderApplication::writeToUniformBuffer(){

    UniformBufferObject ubo;

	glm::vec3 cameraPosition(0, 8, 9);
	ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0,0.7f,0)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.03f, 0.03f, 0.03f));
	ubo.view = glm::lookAt(cameraPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.projection = glm::perspective(glm::radians(45.0f), (float)(resolution.width) / resolution.height, 0.1f, 100.0f);
	ubo.projection[1][1] *= -1;

	ubo.lightDirection = glm::normalize(glm::vec3(2.5f, -2, -3.5));
	ubo.textureParam = 0.77f;
	ubo.cameraPosition = cameraPosition;
	ubo.matColor = glm::vec3(1, 1, 1);

    void* mappedMemory;

    vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &mappedMemory);

    memcpy(mappedMemory, &ubo, sizeof(ubo));

    vkUnmapMemory(device, uniformBufferMemory);

}

void RenderApplication::createDiffuseTexture(){

	Utils::createImageFromFile(
		"resources/images/steel.jpg",	//File name on Disk
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

void RenderApplication::createColorAttachmentImage() {

	Utils::createImage(
		resolution,
		VK_FORMAT_R8G8B8A8_UNORM,	//format
		VK_IMAGE_TILING_OPTIMAL,	//tiling
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,	//usage
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,	//memory properties
		colorAttachmentImage,			//image
		colorAttachmentImageMemory	//image memory
	);
	//Note: no need to transition to layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, render pass will do that for us
}
void RenderApplication::createColorAttachmentImageView() {
	Utils::createImageView(colorAttachmentImage, colorAttachmentImageView, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

void RenderApplication::createDepthAttachmentImage(){

	Utils::createImage(
		resolution,
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

void RenderApplication::createFrameBuffer() {

	std::array<VkImageView, 2> attachments = {
		colorAttachmentImageView,
		depthAttachmentImageView
	};


	VkFramebufferCreateInfo frameBufferInfo = {};
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferInfo.renderPass = renderPass;
	frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	frameBufferInfo.pAttachments = attachments.data();
	frameBufferInfo.width = resolution.width;
	frameBufferInfo.height = resolution.height;
	frameBufferInfo.layers = 1;

	VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferInfo, NULL, &frameBuffer));
}
void RenderApplication::createDescriptorSetLayout() {

    //define a binding for out UBO
    VkDescriptorSetLayoutBinding uniformBufferBinding = {};
    uniformBufferBinding.binding = 0;	//binding = 0
    uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferBinding.descriptorCount = 1;
    uniformBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	//define a binding for our diffuse texture
	VkDescriptorSetLayoutBinding diffuseTextureBinding = {};
	diffuseTextureBinding.binding = 1;
	diffuseTextureBinding.descriptorCount = 1;
	diffuseTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	diffuseTextureBinding.pImmutableSamplers = NULL;
	diffuseTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//define a binding for our environment map
	VkDescriptorSetLayoutBinding environmentMapBinding = {};
	environmentMapBinding.binding = 2;
	environmentMapBinding.descriptorCount = 1;
	environmentMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	environmentMapBinding.pImmutableSamplers = NULL;
	environmentMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


    //put all bindings in an array
    std::array<VkDescriptorSetLayoutBinding, 3> allBindings = {uniformBufferBinding, diffuseTextureBinding, environmentMapBinding };

    //create descriptor set layout for binding to a UBO
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = (uint32_t)allBindings.size(); //number of bindings
    layoutInfo.pBindings = allBindings.data();

    // Create the descriptor set layout.
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout));

}

void RenderApplication::createDescriptorPool(){


    std::array<VkDescriptorPoolSize, 3> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1; // we only need to allocate one descriptor set from the pool.
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

    //Create descriptor pool.
    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool));


}
void RenderApplication::createDescriptorSet() {


    //With the pool allocated, we can now allocate the descriptor set.
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;


    // allocate descriptor set.
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));


    // Specify the uniform buffer info
    VkDescriptorBufferInfo descriptorUniformBufferInfo = {};
    descriptorUniformBufferInfo.buffer = uniformBuffer;
    descriptorUniformBufferInfo.offset = 0;
    descriptorUniformBufferInfo.range = sizeof(UniformBufferObject);

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


    std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;		//binding = 0
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &descriptorUniformBufferInfo;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;		//binding = 1
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &descriptorDiffuseTextureInfo;

	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].dstSet = descriptorSet;
	descriptorWrites[2].dstBinding = 2;		//binding = 2
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pImageInfo = &descriptorEnvironmentMapInfo;

    // perform the update of the descriptor set.
    vkUpdateDescriptorSets(device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, NULL);

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
    commandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &graphicsCommandPool));
}

void RenderApplication::createRenderPass() {

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		//we don't care what the inital layout of the image is
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;	//we want to copy it to a staging buffer and export it after rendering

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

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

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

	
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

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
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

	//Tesselation
	VkPipelineTessellationStateCreateInfo tesselation = {};
	tesselation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tesselation.pNext = NULL;
	tesselation.flags = 0;
	tesselation.patchControlPoints = 3;

    //Viewports
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0;
    viewport.width = (float)resolution.width;
    viewport.height = (float)resolution.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Scissors
    VkRect2D scissor = {};
    scissor.offset = {0,0};
    scissor.extent = resolution;

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


	//Info to create graphics pipeline
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
void RenderApplication::createMainCommandBuffer() {

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;	//no swap chain, so just need one command buffer

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, &mainCommandBuffer));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;

	//main command buffer scope
	VK_CHECK_RESULT(vkBeginCommandBuffer(mainCommandBuffer, &beginInfo));

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffer;
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = resolution;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		//render pass scope
		vkCmdBeginRenderPass(mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			//bind our graphics pipeline
			vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			//bind vertex buffer
			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(mainCommandBuffer, 0, 1, vertexBuffers, offsets);

			//bind index buffer
			vkCmdBindIndexBuffer(mainCommandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			//bind descriptor set
			vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

			//invoke graphics pipeline and draw
			vkCmdDrawIndexed(mainCommandBuffer, (uint32_t)indexArray.size(), 1, 0, 0, 0);

		vkCmdEndRenderPass(mainCommandBuffer);

	VK_CHECK_RESULT(vkEndCommandBuffer(mainCommandBuffer));


}

void RenderApplication::runMainCommandBuffer() {


    //Now we shall finally submit the recorded command buffer to our graphics queue.
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1; // submit a single command buffer
    submitInfo.pCommandBuffers = &mainCommandBuffer; // the command buffer to submit.

    //Create a fence to make the CPU wait for the GPU to finish before proceeding
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &fence));

    //We submit the command buffer on the graphics queue, at the same time giving a fence.
    VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence));

    VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));

    //no longer need fence
    vkDestroyFence(device, fence, NULL);

}

void RenderApplication::exportAsImage(){

	//NOTE: We don't need to transition the color attachment layout image to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL since the render pass already did for us

	Utils::exportImageAsPNG(colorAttachmentImage, resolution, "Rendered Image.png",4);
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

    //free uniform buffer
    vkDestroyBuffer(device, uniformBuffer, NULL);
	vkFreeMemory(device, uniformBufferMemory, NULL);

	//free diffuse texture
	vkDestroyImageView(device, diffuseTextureView, NULL);
	vkDestroyImage(device, diffuseTexture, NULL);
	vkFreeMemory(device, diffuseTextureMemory, NULL);

	//free environment map texture
	vkDestroyImageView(device, environmentMapView, NULL);
	vkDestroyImage(device, environmentMap, NULL);
	vkFreeMemory(device, environmentMapMemory, NULL);

	//free sampler
	vkDestroySampler(device, textureSampler, nullptr);

	//free color attachment image
	vkDestroyImageView(device, colorAttachmentImageView, NULL);
	vkDestroyImage(device, colorAttachmentImage, NULL);
	vkFreeMemory(device, colorAttachmentImageMemory, NULL);

	//free depth attachment image
	vkDestroyImageView(device, depthAttachmentImageView, NULL);
	vkDestroyImage(device, depthAttachmentImage, NULL);
	vkFreeMemory(device, depthAttachmentImageMemory, NULL);

	vkDestroyFramebuffer(device, frameBuffer, NULL);
	vkDestroyRenderPass(device,renderPass, NULL);
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);


	vkDestroyPipeline(device, graphicsPipeline, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyCommandPool(device, graphicsCommandPool, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);

}

VkCommandPool& RenderApplication::getTransferCmdPool(){
	return graphicsCommandPool;
}
VkQueue& RenderApplication::getTransferQueue(){
	return graphicsQueue;
}
