#include <RenderApplication.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#endif


VkExtent2D RenderApplication::resolution = {900,900};
VkInstance RenderApplication::instance;
VkDebugReportCallbackEXT RenderApplication::debugReportCallback;
VkPhysicalDevice RenderApplication::physicalDevice;
VkDevice RenderApplication::device;
VkBuffer RenderApplication::vertexBuffer;
VkDeviceMemory RenderApplication::vertexBufferMemory;
VkBuffer RenderApplication::indexBuffer;
VkDeviceMemory RenderApplication::indexBufferMemory;
VkBuffer RenderApplication::uniformBuffer;
VkDeviceMemory RenderApplication::uniformBufferMemory;
VkImage RenderApplication::colorImage;
VkDeviceMemory RenderApplication::colorImageMemory;
VkImageView RenderApplication::colorImageView;
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


const std::vector<const char *> RenderApplication::requiredLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};
const std::vector<const char *> RenderApplication::requiredInstanceExtensions = {
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME
};

//Hard coded vertex and index buffers
const Vertex singleTriangle[3] = {
	{{0.0, -0.85f,0},{0,0,1},{0,0}},
	{{0.8f, 0.7f,0},{0,1,0},{0,0}},
	{{-0.8f, 0.7f,0},{1,0,0},{0,0}}
};

const uint32_t triangleIndices[3] = {0,1,2};

void RenderApplication::run() {

    
    // Initialize vulkan
    createInstance();
    findPhysicalDevice();
    createDevice();
    

    //create descriptor and command resources
    createDescriptorSetLayout();
    createDescriptorPool();
    createCommandPool();


    createVertexBuffer();
    writeToVertexBuffer();

    createIndexBuffer();
    writeToIndexBuffer();

    createUniformBuffer();
    writeToUniformBuffer();

	createColorImage();
	createColorImageView();
	
	

	//create descriptors 
    createDescriptorSet();

    //for graphics
	createRenderPass();
	createFrameBuffer();
    createGraphicsPipeline();

    //record command buffer
    createMainCommandBuffer();

    // Finally, run the recorded command buffer.
    runMainCommandBuffer();

	exportAsImage();

    // Clean up all Vulkan resources.
    cleanup();
}


void RenderApplication::createInstance() {
    

	//Make sure all required instance layers/extensions are supported and throw a runtime error if not
	if (enableValidationLayers) {

		//Check for presence of required layers
		uint32_t numAvailableLayers;

		vkEnumerateInstanceLayerProperties(&numAvailableLayers, NULL);
		std::vector<VkLayerProperties> allAvailableLayerProps(numAvailableLayers);
		vkEnumerateInstanceLayerProperties(&numAvailableLayers, allAvailableLayerProps.data());

		for (const char* currRequiredLayer : requiredLayers) {

			bool foundRequiredLayer = false;
			for (VkLayerProperties currAvailableLayerProp : allAvailableLayerProps) {
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
        std::vector<VkExtensionProperties> allAvailableExtensionProps(numAvailableExtensions);
        vkEnumerateInstanceExtensionProperties(NULL, &numAvailableExtensions, allAvailableExtensionProps.data());


		for (const char* currRequiredExtension : requiredInstanceExtensions) {

			bool foundRequiredExtension = false;
			for (VkExtensionProperties currAvailableExtProp : allAvailableExtensionProps) {
				if (strcmp(currRequiredExtension, currAvailableExtProp.extensionName) == 0) {
					foundRequiredExtension = true;
					break;
				}			
			}
			if (!foundRequiredExtension) {
				string errorMessage = "Extension " + string(currRequiredExtension) + " not supported\n";
				throw std::runtime_error(errorMessage);
			}
		}

    }//End if(enableValidationLayers)


    //Next, we actually create the instance.
    
    

    //Contains application info. This is actually not that important.
    //The only real important field is apiVersion.
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Vulkan Compute Application";
    applicationInfo.applicationVersion = 0;
    applicationInfo.pEngineName = "ComputeEngine";
    applicationInfo.engineVersion = 0;
    applicationInfo.apiVersion = VK_API_VERSION_1_1;;
    
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &applicationInfo;
    

    // Give our desired instance layers and extensions to vulkan.
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = (uint32_t)requiredLayers.size();
		createInfo.ppEnabledLayerNames = requiredLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

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

	throw std::runtime_error("Could not load find a valid physical device for our operations");
}

bool RenderApplication::isValidPhysicalDevice(VkPhysicalDevice potentialPhysicalDevice, uint32_t &familyIndex) {

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(potentialPhysicalDevice, &supportedFeatures);

	//We would normally check the supportedFeatures structure to see that all our device features are supported
	//by this potential physical device. Since we have none, we just move on


    //If we had any required device extensions, we would check if they are supported by the physical device here before
    //supplying them to the logical device create info.

	//lastly, we make sure there exists a queue family index that supports the operations we want
	familyIndex = getQueueFamilyIndex(potentialPhysicalDevice);
	return familyIndex != -1;


}
// Returns the index of a queue family that supports compute and graphics operations. 
uint32_t RenderApplication::getQueueFamilyIndex(VkPhysicalDevice currPhysicalDevice) {

    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(currPhysicalDevice, &queueFamilyCount, NULL);

    // Retrieve all queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(currPhysicalDevice, &queueFamilyCount, queueFamilies.data());

    // Now find a family that supports compute.
    uint32_t currFamilyIndex;
    for (currFamilyIndex = 0; currFamilyIndex < queueFamilies.size(); ++currFamilyIndex) {
        VkQueueFamilyProperties currFamily = queueFamilies[currFamilyIndex];

        if ((currFamily.queueCount > 0) && 
			/*(currFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) &&*/
			(currFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			(currFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)) {

            // found a queue family with transfer and graphics. We're done!
            break;
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

    // Specify any desired device features here. We do not need any for this application, though.
    VkPhysicalDeviceFeatures deviceFeatures = {};

    //Now we create the logical device. The logical device allows us to interact with the physical device.
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; // when creating the logical device, we also specify what queues it has.
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	if (enableValidationLayers) {
		deviceCreateInfo.enabledLayerCount = (uint32_t)requiredLayers.size();  // need to specify validation layers here as well.
		deviceCreateInfo.ppEnabledLayerNames = requiredLayers.data();
	} else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	// no device extensions for this app since we aren't using a swapchain
	deviceCreateInfo.enabledExtensionCount = 0;

    VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device)); // create logical device.

	//the index within this queue family of the queue to retrieve, we just take the first one
    uint32_t particularQueueIndex = 0;	

    // Get a handle to the only member of the queue family.
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, particularQueueIndex, &graphicsQueue);
}

void RenderApplication::createVertexBuffer(){

	Utils::createBuffer(
		sizeof(singleTriangle),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer, vertexBufferMemory
	);

}

void RenderApplication::writeToVertexBuffer(){


	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	//create staging buffer
	Utils::createBuffer(
		sizeof(singleTriangle),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		stagingBuffer, stagingBufferMemory
	);

	//copy contents of our hard coded triangle into the staging buffer
	void* mappedMemory;
	vkMapMemory(device, stagingBufferMemory, 0, sizeof(singleTriangle), 0, &mappedMemory);
	memcpy(mappedMemory, singleTriangle, sizeof(singleTriangle));
	vkUnmapMemory(device, stagingBufferMemory);

	//copy contents of staging buffer to vertex buffer
	Utils::copyBuffer(stagingBuffer, vertexBuffer, sizeof(singleTriangle));

	//destroy staging buffer
	vkDestroyBuffer(device, stagingBuffer, NULL);
	vkFreeMemory(device, stagingBufferMemory, NULL);
}

void RenderApplication::createIndexBuffer(){

	Utils::createBuffer(
		sizeof(triangleIndices),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer, indexBufferMemory
	);
}

void RenderApplication::writeToIndexBuffer(){

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	//create staging buffer
	Utils::createBuffer(
		sizeof(triangleIndices),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		stagingBuffer, stagingBufferMemory
	);

	//copy contents of our hard coded triangle indices into the staging buffer
	void* mappedMemory;
	vkMapMemory(device, stagingBufferMemory, 0, sizeof(triangleIndices), 0, &mappedMemory);
	memcpy(mappedMemory, triangleIndices, sizeof(triangleIndices));
	vkUnmapMemory(device, stagingBufferMemory);

	//copy contents of staging buffer to index buffer
	Utils::copyBuffer(stagingBuffer, indexBuffer, sizeof(triangleIndices));

	//destroy staging buffer
	vkDestroyBuffer(device, stagingBuffer, NULL);
	vkFreeMemory(device, stagingBufferMemory, NULL);
}

void RenderApplication::createUniformBuffer(){

	Utils::createBuffer(
		sizeof(UniformBufferObject), 
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
		uniformBuffer, uniformBufferMemory);

}

void RenderApplication::writeToUniformBuffer(){

    UniformBufferObject ubo;
	
	ubo.model = glm::mat4(1.0f);
	ubo.view = glm::lookAt(glm::vec3(0, 0, 100), glm::vec3(0,0,0), glm::vec3(0, 1, 0));
	ubo.projection = glm::perspective(glm::radians(45.0f), (float)(resolution.height) / resolution.width, 0.0f, 1000.0f);


    void* mappedMemory;

    vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &mappedMemory);

    memcpy(mappedMemory, &ubo, sizeof(ubo));

    vkUnmapMemory(device, uniformBufferMemory);

}


void RenderApplication::createColorImage() {

	Utils::createImage(
		resolution.width,	//width
		resolution.height,	//height
		VK_FORMAT_R8G8B8A8_UNORM,	//format
		VK_IMAGE_TILING_OPTIMAL,	//tiling
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,	//usage
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,	//memory properties
		colorImage,			//image
		colorImageMemory	//image memory
	);

	//Note: no need to transition to layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, render pass will do that for us
}
void RenderApplication::createColorImageView() {
	Utils::createImageView(colorImage, colorImageView, VK_FORMAT_R8G8B8A8_UNORM);
}

void RenderApplication::createFrameBuffer() {

	VkFramebufferCreateInfo frameBufferInfo = {};
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferInfo.renderPass = renderPass;
	frameBufferInfo.attachmentCount = 1;
	frameBufferInfo.pAttachments = &colorImageView;
	frameBufferInfo.width = resolution.width;
	frameBufferInfo.height = resolution.height;
	frameBufferInfo.layers = 1;

	VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferInfo, NULL, &frameBuffer));
}
void RenderApplication::createDescriptorSetLayout() {



    //define a binding for a UBO
    VkDescriptorSetLayoutBinding uniformBufferBinding = {};
    uniformBufferBinding.binding = 0;	//binding = 0
    uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferBinding.descriptorCount = 1;
    uniformBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	

    //put all bindings in an array
    std::array<VkDescriptorSetLayoutBinding, 1> allBindings = {uniformBufferBinding};

    //create descriptor set layout for binding to a UBO
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = (uint32_t)allBindings.size(); //number of bindings
    descriptorSetLayoutCreateInfo.pBindings = allBindings.data();

    // Create the descriptor set layout. 
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout));
	
}

void RenderApplication::createDescriptorPool(){


	
    std::array<VkDescriptorPoolSize, 1> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
	

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


    std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;		//binding = 0
    descriptorWrites[0].dstArrayElement = 0;	
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &descriptorUniformBufferInfo;


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
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;	//we want to copy it to a staging buffer and export it after rendering

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

}

void RenderApplication::createGraphicsPipeline(){
    
	//create vertex shader module
	VkShaderModule vertexShaderModule = Utils::createShaderModule("resources/shaders/vert.spv");
	
	//create fragment shader module
	VkShaderModule fragmentShaderModule = Utils::createShaderModule("resources/shaders/frag.spv");


	//Vertex Shader Stage
	VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertexShaderModule;
	vertexShaderStageInfo.pName = "main";
	
	//Fragment Shader Stage
	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragmentShaderModule;
	fragmentShaderStageInfo.pName = "main";

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
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	//Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


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

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = NULL;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = NULL;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	//Create Graphics Pipeline 
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline));


	//destroy shader modules 
	vkDestroyShaderModule(device,vertexShaderModule, NULL);
	vkDestroyShaderModule(device, fragmentShaderModule, NULL);
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

		VkClearValue clearColor = { 0.0f,0.0f,0.0f,1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		//render pass scope
		vkCmdBeginRenderPass(mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			//bind our graphics pipeline
			vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			//bind vertex buffer
			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(mainCommandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(mainCommandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
			
			vkCmdDraw(mainCommandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(mainCommandBuffer);

	VK_CHECK_RESULT(vkEndCommandBuffer(mainCommandBuffer));

   
}

void RenderApplication::runMainCommandBuffer() {


    //Now we shall finally submit the recorded command buffer to a the compute queue.
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

    //We submit the command buffer on the queue, at the same time giving a fence.
    VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence));

    VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));

    //no longer need fence
    vkDestroyFence(device, fence, NULL);

}

void RenderApplication::exportAsImage() {

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VkDeviceSize bufferByteSize = resolution.width * resolution.height * 4;

	Utils::createBuffer(bufferByteSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

	//NOTE: We don't need to transition the color attachment image to transfer src since the render pass already did for us
	Utils::copyImageToBuffer(stagingBuffer, colorImage, resolution.width, resolution.height);

	void* mappedMemory;
	vkMapMemory(device, stagingBufferMemory, 0, bufferByteSize, 0, &mappedMemory);

	//write output image to disk as a png
	stbi_write_png("Rendered Image.png", resolution.width, resolution.height, 4, mappedMemory, resolution.width * 4);

	//write pixels to another array
	vkUnmapMemory(device, stagingBufferMemory);


	//Clean Up Staging Buffer
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

}
void RenderApplication::cleanup() {

	//clean up all Vulkan resources
	if (enableValidationLayers) {
		// destroy callback.
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func == nullptr) {
			throw std::runtime_error("Could not load vkDestroyDebugReportCallbackEXT");
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

	//free color image
	vkDestroyImageView(device, colorImageView, NULL);
	vkDestroyImage(device, colorImage, NULL);
	vkFreeMemory(device, colorImageMemory, NULL);

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
