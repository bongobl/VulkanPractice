#include <ComputeApplication.h>
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

uint32_t ComputeApplication::IMAGE_WIDTH = -1;
uint32_t ComputeApplication::IMAGE_HEIGHT = -1;
VkDeviceSize ComputeApplication::imageSize;
unsigned char* ComputeApplication::inputImageData;
VkInstance ComputeApplication::instance;
VkDebugReportCallbackEXT ComputeApplication::debugReportCallback;
VkPhysicalDevice ComputeApplication::physicalDevice;
VkDevice ComputeApplication::device;
VkImage ComputeApplication::inputImage;
VkDeviceMemory ComputeApplication::inputImageMemory;
VkImageView ComputeApplication::inputImageView;
VkBuffer ComputeApplication::uniformBuffer;
VkDeviceMemory ComputeApplication::uniformBufferMemory;
VkImage ComputeApplication::outputImage;
VkDeviceMemory ComputeApplication::outputImageMemory;
VkImageView ComputeApplication::outputImageView;
VkDescriptorPool ComputeApplication::descriptorPool;
VkDescriptorSet ComputeApplication::descriptorSet;
VkDescriptorSetLayout ComputeApplication::descriptorSetLayout;
VkShaderModule ComputeApplication::computeShaderModule;
VkPipeline ComputeApplication::computePipeline;
VkPipelineLayout ComputeApplication::pipelineLayout;
VkCommandPool ComputeApplication::commandPool;
VkCommandBuffer ComputeApplication::mainCommandBuffer;
uint32_t ComputeApplication::queueFamilyIndex;
VkQueue ComputeApplication::queue;


const std::vector<const char *> ComputeApplication::requiredLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};
const std::vector<const char *> ComputeApplication::requiredInstanceExtensions = {
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

void ComputeApplication::run() {

    
    // Initialize vulkan
    createInstance();
    findPhysicalDevice();
    createDevice();
    
    //also sets the width and height
    loadImage();

    //create descriptor and command resources
    createDescriptorSetLayout();
    createDescriptorPool();
    createCommandPool();


    //create and init GPU resources
    createInputImage();
	writeToInputImage();
	createInputImageView();

    createUniformBuffer();
    writeToUniformBuffer();

	createOutputImage();
	createOutputImageView();

	//create descriptors 
    createDescriptorSet();

    //create pipeline
    createComputePipeline();

    //record command buffer
    createMainCommandBuffer();

    // Finally, run the recorded command buffer.
    runCommandBuffer();

    // Save that buffer as a png on disk.
	exportOutputImage();

    // Clean up all Vulkan resources.
    cleanup();
}

void ComputeApplication::loadImage(){

	string imageName = "resources/images/oahu.jpg";

    //read in the file here
    int numChannels = -1;
    int imageWidth, imageHeight;

    //load image
    inputImageData = stbi_load(imageName.c_str(), &imageWidth, &imageHeight, &numChannels, STBI_rgb_alpha);
    if (numChannels == -1) {
        std::string error =  "Compute Application::loadImage: failed to load image " + imageName + "\n";
        throw std::runtime_error(error.c_str());
    }

    cout << "loaded " << imageName << endl;
    cout << "Num numChannels: " << numChannels << endl;
    cout << "Width: " << imageWidth << endl << "Height: " << imageHeight << endl;

    IMAGE_WIDTH = imageWidth;
    IMAGE_HEIGHT = imageHeight;

	imageSize = IMAGE_WIDTH * IMAGE_HEIGHT * 4;
}

void ComputeApplication::exportOutputImage() {

	//Buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Utils::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

	Utils::transitionImageLayout(outputImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	Utils::copyImageToBuffer(stagingBuffer, outputImage, IMAGE_WIDTH, IMAGE_HEIGHT);

	void* mappedMemory;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &mappedMemory);

	//write output image to disk as a png
	stbi_write_png("Simple Image.png", IMAGE_WIDTH, IMAGE_HEIGHT, 4, mappedMemory, IMAGE_WIDTH * 4);

	//write pixels to another array
	vkUnmapMemory(device, stagingBufferMemory);

	//Clean Up Staging Buffer
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

}
void ComputeApplication::createInstance() {
    

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

void ComputeApplication::findPhysicalDevice() {
    
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

bool ComputeApplication::isValidPhysicalDevice(VkPhysicalDevice potentialPhysicalDevice, uint32_t &familyIndex) {

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(potentialPhysicalDevice, &supportedFeatures);

	//We would normally check the supportedFeatures structure to see that all our features are supported
	//by this potential physical device. Since we have none, we just choose this device and get the queue family we need

	familyIndex = getQueueFamilyIndex(potentialPhysicalDevice);
	return familyIndex != -1;


}
// Returns the index of a queue family that supports compute and graphics operations. 
uint32_t ComputeApplication::getQueueFamilyIndex(VkPhysicalDevice currPhysicalDevice) {

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

void ComputeApplication::createDevice() {


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


void ComputeApplication::createInputImage(){
    
	Utils::createImage(
		IMAGE_WIDTH,		//Width
		IMAGE_HEIGHT,		//Height
		VK_FORMAT_R8G8B8A8_UNORM,	//Format
		VK_IMAGE_TILING_OPTIMAL,	//Tiling
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT,	//Usage
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,	//Memory properties
		inputImage,			//image
		inputImageMemory	//image memory
	);	

}

void ComputeApplication::writeToInputImage() {

	//Buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;


	Utils::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

	void* mappedMemory;
    
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &mappedMemory);
	memcpy(mappedMemory, inputImageData, imageSize);
	vkUnmapMemory(device, stagingBufferMemory);

	Utils::transitionImageLayout(inputImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Utils::copyBufferToImage(stagingBuffer, inputImage, IMAGE_WIDTH, IMAGE_HEIGHT);
	Utils::transitionImageLayout(inputImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);


	//Clean Up Staging Buffer
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}
void ComputeApplication::createInputImageView() {

	Utils::createImageView(inputImage, inputImageView);
}




void ComputeApplication::createUniformBuffer(){

	Utils::createBuffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffer, uniformBufferMemory);

}


void ComputeApplication::writeToUniformBuffer(){

    UniformBufferObject ubo;
	
	ubo.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	
    ubo.width = IMAGE_WIDTH;
    ubo.height = IMAGE_HEIGHT;
    ubo.saturation = 1.5;
    ubo.blur = 51;

    void* mappedMemory;

    vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &mappedMemory);

    memcpy(mappedMemory, &ubo, sizeof(ubo));

    vkUnmapMemory(device, uniformBufferMemory);

}


void ComputeApplication::createOutputImage() {
	
	Utils::createImage(
		IMAGE_WIDTH,		//Width
		IMAGE_HEIGHT,		//Height
		VK_FORMAT_R8G8B8A8_UNORM,	//Format
		VK_IMAGE_TILING_OPTIMAL,	//Tiling
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,	//Usage
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,	//Memory properties
		outputImage,			//image
		outputImageMemory	//image memory
	);

	//shader needs image to be in general layout
	Utils::transitionImageLayout(outputImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

}

void ComputeApplication::createOutputImageView() {

	Utils::createImageView(outputImage, outputImageView);
}

void ComputeApplication::createDescriptorSetLayout() {


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
}

void ComputeApplication::createDescriptorPool(){

    //So we will allocate a descriptor set here.
    //But we need to first create a descriptor pool to do that. 
   
    //Our descriptor pool can only allocate a single storage buffer.
   
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
}
void ComputeApplication::createDescriptorSet() {
    

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
    descriptorWrites[1].dstArrayElement = 0;	//??????
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
}


void ComputeApplication::createComputePipeline() {

    
    //Create a shader module. A shader module basically just encapsulates some shader code.
    std::vector<char> shaderCode = Utils::readFile("resources/shaders/comp.spv");
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
    createInfo.codeSize = shaderCode.size();
    VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, NULL, &computeShaderModule));

    /*
    Now let us actually create the compute pipeline.
    A compute pipeline is very simple compared to a graphics pipeline.
    It only consists of a single stage with a compute shader. 

    So first we specify the compute shader stage, and it's entry point(main).
    */
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageCreateInfo.module = computeShaderModule;
    shaderStageCreateInfo.pName = "main";

    
    //The pipeline layout allows the pipeline to access descriptor sets. 
    //So we just specify the descriptor set layout we created earlier.
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout; 
    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout));

    VkComputePipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStageCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;

    
    //Now, we finally create the compute pipeline. 
    VK_CHECK_RESULT(vkCreateComputePipelines( device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &computePipeline));

    //don't need shader module anymore for any other pipeline, so destroy
    vkDestroyShaderModule(device, computeShaderModule, NULL);
}

void ComputeApplication::createCommandPool(){

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

void ComputeApplication::createMainCommandBuffer() {
    
    
    //Now allocate a command buffer from the command pool. 
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool; // specify the command pool to allocate from. 

    // if the command buffer is primary, it can be directly submitted to queues. 
    // A secondary buffer has to be called from some primary command buffer, and cannot be directly 
    // submitted to a queue. To keep things simple, we use a primary command buffer. 
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1; // allocate a single command buffer. 
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &mainCommandBuffer)); // allocate command buffer.

    /*
    Now we shall start recording commands into the newly allocated command buffer. 
    */
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // the buffer is only submitted and used once in this application.
    VK_CHECK_RESULT(vkBeginCommandBuffer(mainCommandBuffer, &beginInfo)); // start recording commands.

    
    //We need to bind a pipeline, AND a descriptor set before we dispatch.

    //The validation layer will NOT give warnings if you forget these, so be very careful not to forget them.
    vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

    
    //Calling vkCmdDispatch starts the compute pipeline, and executes the compute shader.
    //The number of workgroups is specified in the arguments.
    //If you are already familiar with compute shaders from OpenGL, this should be nothing new to you.
    vkCmdDispatch(mainCommandBuffer, (uint32_t)ceil(IMAGE_WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(IMAGE_HEIGHT / float(WORKGROUP_SIZE)), 1);

    VK_CHECK_RESULT(vkEndCommandBuffer(mainCommandBuffer)); // end recording commands.
}

void ComputeApplication::runCommandBuffer() {

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


    /*The command will not have finished executing until the fence is signalled.
    So we wait here.
    Directly after this, we will copy our output image to a staging buffer and export it
    and we will not be sure that the command has finished executing unless we wait for the fence.
    Hence, we use a fence here.*/
    VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));

    //no longer need fence
    vkDestroyFence(device, fence, NULL);
}

void ComputeApplication::cleanup() {

	//clean up all Vulkan resources

    if (enableValidationLayers) {
        // destroy callback.
        auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
        if (func == nullptr) {
            throw std::runtime_error("Could not load vkDestroyDebugReportCallbackEXT");
        }
        func(instance, debugReportCallback, NULL);
    }

    //free CPU image buffer
   	stbi_image_free(inputImageData);

    //free uniform buffer
    vkFreeMemory(device, uniformBufferMemory, NULL);
    vkDestroyBuffer(device, uniformBuffer, NULL);

	//free input Image
	vkFreeMemory(device, inputImageMemory, NULL);
	vkDestroyImage(device, inputImage, NULL);
	vkDestroyImageView(device, inputImageView, NULL);

	//free output Image
	vkFreeMemory(device, outputImageMemory, NULL);
	vkDestroyImage(device, outputImage, NULL);
	vkDestroyImageView(device, outputImageView, NULL);

    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyPipeline(device, computePipeline, NULL);
    vkDestroyCommandPool(device, commandPool, NULL);        
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);      
	
}
