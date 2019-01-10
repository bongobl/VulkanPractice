#include <RenderApplication.h>
#include <Utils.h>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#endif

#include <fstream>


VkInstance RenderApplication::instance;
VkDebugReportCallbackEXT RenderApplication::debugReportCallback;
VkPhysicalDevice RenderApplication::physicalDevice;
VkDevice RenderApplication::device;
VkBuffer RenderApplication::uniformBuffer;
VkDeviceMemory RenderApplication::uniformBufferMemory;
VkDescriptorPool RenderApplication::descriptorPool;
VkDescriptorSet RenderApplication::descriptorSet;
VkDescriptorSetLayout RenderApplication::descriptorSetLayout;
VkPipelineLayout RenderApplication::pipelineLayout;
VkRenderPass RenderApplication::renderPass;
VkCommandPool RenderApplication::commandPool;
VkCommandBuffer RenderApplication::mainCommandBuffer;
uint32_t RenderApplication::queueFamilyIndex;
VkQueue RenderApplication::queue;


const std::vector<const char *> RenderApplication::requiredLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};
const std::vector<const char *> RenderApplication::requiredInstanceExtensions = {
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME
};




struct Color {
	float r, g, b, a;
};

struct UniformBufferObject{
    
	Color color;

    uint32_t width;
    uint32_t height;
    float saturation;
    int32_t blur;
};

void RenderApplication::run() {

    
    // Initialize vulkan
    createInstance();
    findPhysicalDevice();
    createDevice();
    

    //create descriptor and command resources
    createDescriptorSetLayout();
    createDescriptorPool();
    createCommandPool();


    createUniformBuffer();
    writeToUniformBuffer();


	//create descriptors 
    createDescriptorSet();

    //for graphics
	createRenderPass();
    createGraphicsPipeline();

    //record command buffer
    createMainCommandBuffer();

    // Finally, run the recorded command buffer.
    runCommandBuffer();

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

		if (isValidPhysicalDevice(currPhysicalDevice, queueFamilyIndex)) {
			physicalDevice = currPhysicalDevice;
			return;
		}	
    }

	throw std::runtime_error("Could not load find a valid physical device for our operations");
}

bool RenderApplication::isValidPhysicalDevice(VkPhysicalDevice potentialPhysicalDevice, uint32_t &familyIndex) {

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(potentialPhysicalDevice, &supportedFeatures);

	//We would normally check the supportedFeatures structure to see that all our features are supported
	//by this potential physical device. Since we have none, we just choose this device and get the queue family we need

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
			(currFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && 
			(currFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {

            // found a queue family with compute and graphics. We're done!
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
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
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
    vkGetDeviceQueue(device, queueFamilyIndex, particularQueueIndex, &queue);
}

void RenderApplication::createUniformBuffer(){

	Utils::createBuffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffer, uniformBufferMemory);

}


void RenderApplication::writeToUniformBuffer(){

    UniformBufferObject ubo;
	
	ubo.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	
    ubo.width = 4;	//dubby val
    ubo.height = 4;	//dummy val
    ubo.saturation = 1.5;
    ubo.blur = 51;

    void* mappedMemory;

    vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &mappedMemory);

    memcpy(mappedMemory, &ubo, sizeof(ubo));

    vkUnmapMemory(device, uniformBufferMemory);

}

void RenderApplication::createDescriptorSetLayout() {

	/*
	//define a storage image binding for our input image
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;	//binding = 0
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	samplerLayoutBinding.pImmutableSamplers = NULL;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    //define a binding for a UBO
    VkDescriptorSetLayoutBinding uniformBufferBinding = {};
    uniformBufferBinding.binding = 1;	//binding = 1
    uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferBinding.descriptorCount = 1;
    uniformBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	//define a storage image binding for our output image
	VkDescriptorSetLayoutBinding outputImageBinding = {};
	outputImageBinding.binding = 2;		//binding = 2
	outputImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	outputImageBinding.descriptorCount = 1;
	outputImageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	

    //put all bindings in an array
    std::array<VkDescriptorSetLayoutBinding, 3> allBindings = { samplerLayoutBinding, uniformBufferBinding, outputImageBinding};

    //create descriptor set layout for binding to a storage buffer, UBO and another storage buffer
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = (uint32_t)allBindings.size(); //number of bindings
    descriptorSetLayoutCreateInfo.pBindings = allBindings.data();

    // Create the descriptor set layout. 
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout));
	*/
}

void RenderApplication::createDescriptorPool(){


	/*
    std::array<VkDescriptorPoolSize, 3> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = 1;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[2].descriptorCount = 1;
	

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1; // we only need to allocate one descriptor set from the pool.
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

    //Create descriptor pool.
    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool));

	*/
}
void RenderApplication::createDescriptorSet() {
    
	/*
    //With the pool allocated, we can now allocate the descriptor set. 
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO; 
    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

    // allocate descriptor set.
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

	// Specify the input image info
	VkDescriptorImageInfo inputImageInfo = {};
	inputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	inputImageInfo.imageView = inputImageView;
	
    // Specify the uniform buffer info
    VkDescriptorBufferInfo descriptorUniformBufferInfo = {};
    descriptorUniformBufferInfo.buffer = uniformBuffer;
    descriptorUniformBufferInfo.offset = 0;
    descriptorUniformBufferInfo.range = sizeof(UniformBufferObject);

	// Specify the output image info
	VkDescriptorImageInfo outputImageInfo = {};
	outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	outputImageInfo.imageView = outputImageView;


    std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pImageInfo = &inputImageInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;	
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &descriptorUniformBufferInfo;

	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].dstSet = descriptorSet;
	descriptorWrites[2].dstBinding = 2;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pImageInfo = &outputImageInfo;


    // perform the update of the descriptor set.
    vkUpdateDescriptorSets(device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, NULL);
	*/
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
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool));
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
	auto vertexShaderCode = Utils::readFile("resources/shaders/vert.spv");
	VkShaderModule vertexShaderModule = Utils::createShaderModule(vertexShaderCode);
	
	//create fragment shader module
	auto fragmentShaderCode = Utils::readFile("resources/shaders/frag.spv");
	VkShaderModule fragmentShaderModule = Utils::createShaderModule(fragmentShaderCode);



	VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertexShaderModule;
	vertexShaderStageInfo.pName = "main";
	
	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragmentShaderModule;
	fragmentShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

	


	//destroy shader modules
	vkDestroyShaderModule(device,vertexShaderModule, NULL);
	vkDestroyShaderModule(device, fragmentShaderModule, NULL);
}
void RenderApplication::createMainCommandBuffer() {
    
    
   
}

void RenderApplication::runCommandBuffer() {

	/* THIS CODE PROBABLY DOESN'T CHANGE AT ALL, ENABLE WHEN READY

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
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));

    VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));

    //no longer need fence
    vkDestroyFence(device, fence, NULL);
	*/
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


    //free uniform buffer
    vkFreeMemory(device, uniformBufferMemory, NULL);
    vkDestroyBuffer(device, uniformBuffer, NULL);


	vkDestroyRenderPass(device,renderPass, NULL);
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyCommandPool(device, commandPool, NULL);        
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);      
	
}
