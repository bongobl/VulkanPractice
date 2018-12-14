#include "../include/ComputeApplication.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#endif

#include <fstream>

uint32_t ComputeApplication::OUTPUT_WIDTH = -1;
uint32_t ComputeApplication::OUTPUT_HEIGHT = -1;

// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f)                                                                              \
{                                                                                                       \
    VkResult res = (f);                                                                                 \
    if (res != VK_SUCCESS)                                                                              \
    {                                                                                                   \
        printf("Fatal : VkResult is %d in %s at line %d\n", res,  __FILE__, __LINE__); \
        assert(res == VK_SUCCESS);                                                                      \
    }                                                                                                   \
}

struct UniformBufferObject{
    float brightness;
    float colorR;
    float colorG;
    float colorB;

    uint32_t width;
    uint32_t height;
    uint32_t padding1;
    uint32_t padding2;
};

void ComputeApplication::run() {

    
    // Initialize vulkan
    createInstance();
    findPhysicalDevice();
    createDevice();
    
    //also sets the width and height
    loadImage();

    //create and init buffer resources on GPU
    createStorageBuffer();
    writeToStorageBuffer();
    createUniformBuffer();
    writeToUniformBuffer();

    //create descriptor resources
    createDescriptorSetLayout();
    createDescriptorSet();

    //create pipeline
    createComputePipeline();

    //record command buffer
    createCommandBuffer();

    // Finally, run the recorded command buffer.
    runCommandBuffer();

    // Save that buffer as a png on disk.
    saveRenderedImage();

    // Clean up all Vulkan resources.
    cleanup();
}

void ComputeApplication::loadImage(){

	string imageName = "resources/images/vulkan-logo.png";

    //read in the file here
    int numChannels = -1;
    int imageWidth, imageHeight;

    //load image
    inputImageData = stbi_load(imageName.c_str(), &imageWidth, &imageHeight, &numChannels, STBI_rgb_alpha);
    if (numChannels == -1) {
        std::string error =  "Compute Application::loadImage: failed to load image " + imageName + "\n";
        throw std::runtime_error(error.c_str());
    }
    cout << "Num numChannels: " << numChannels << endl;
    cout << "Width: " << imageWidth << endl << "Height: " << imageHeight << endl;

    OUTPUT_WIDTH = imageWidth;
    OUTPUT_HEIGHT = imageHeight;

    storageBufferSize = sizeof(Pixel) * OUTPUT_WIDTH * OUTPUT_HEIGHT;

}

void ComputeApplication::saveRenderedImage() {
    void* mappedMemory = NULL;
    
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(device, storageBufferMemory, 0, storageBufferSize, 0, &mappedMemory);
    Pixel* pmappedMemory = (Pixel *)mappedMemory;

    // Get the color data from the buffer, and cast it to bytes.
    // We save the data to a vector.
    std::vector<unsigned char> image;
    image.reserve(OUTPUT_WIDTH * OUTPUT_HEIGHT * 4);
    for (int i = 0; i < OUTPUT_WIDTH * OUTPUT_HEIGHT; i += 1) {
        //image.push_back((unsigned char)(255.0f * (pmappedMemory[i].r)));
        //image.push_back((unsigned char)(255.0f * (pmappedMemory[i].g)));
        ///image.push_back((unsigned char)(255.0f * (pmappedMemory[i].b)));
        //image.push_back((unsigned char)(255.0f * (pmappedMemory[i].a)));

    	//use this when you don't have to put back in range of 0-255
        image.push_back((unsigned char)pmappedMemory[i].r);
        image.push_back((unsigned char)pmappedMemory[i].g);
        image.push_back((unsigned char)pmappedMemory[i].b);
        image.push_back((unsigned char)pmappedMemory[i].a);
    }
    // Done reading, so unmap.
    vkUnmapMemory(device, storageBufferMemory);

    // Now we save the acquired color data to a .png.
    stbi_write_png("Simple Image.png", OUTPUT_WIDTH, OUTPUT_HEIGHT, 4, image.data(), OUTPUT_WIDTH * 4);
}



void ComputeApplication::createInstance() {
    std::vector<const char *> enabledExtensions;

    /*
    By enabling validation layers, Vulkan will emit warnings if the API
    is used incorrectly. We shall enable the layer VK_LAYER_LUNARG_standard_validation,
    which is basically a collection of several useful validation layers.
    */
    if (enableValidationLayers) {
        /*
        We get all supported layers with vkEnumerateInstanceLayerProperties.
        */
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, NULL);

        std::vector<VkLayerProperties> layerProperties(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

        /*
        And then we simply check if VK_LAYER_LUNARG_standard_validation is among the supported layers.
        */
        bool foundLayer = false;
        for (VkLayerProperties prop : layerProperties) {
            
            if (strcmp("VK_LAYER_LUNARG_standard_validation", prop.layerName) == 0) {
                foundLayer = true;
                break;
            }

        }
        
        if (!foundLayer) {
            throw std::runtime_error("Layer VK_LAYER_LUNARG_standard_validation not supported\n");
        }
        enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation"); // Alright, we can use this layer.

        /*
        We need to enable an extension named VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
        in order to be able to print the warnings emitted by the validation layer.

        So again, we just check if the extension is among the supported extensions.
        */
        
        uint32_t extensionCount;
        
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
        std::vector<VkExtensionProperties> extensionProperties(extensionCount);
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties.data());

        bool foundExtension = false;
        for (VkExtensionProperties prop : extensionProperties) {
            if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) == 0) {
                foundExtension = true;
                break;
            }

        }

        if (!foundExtension) {
            throw std::runtime_error("Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n");
        }
        enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }//End if(enableValidationLayers)


    /*
    Next, we actually create the instance.
    
    */
    
    /*
    Contains application info. This is actually not that important.
    The only real important field is apiVersion.
    */
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Hello world app";
    applicationInfo.applicationVersion = 0;
    applicationInfo.pEngineName = "awesomeengine";
    applicationInfo.engineVersion = 0;
    applicationInfo.apiVersion = VK_API_VERSION_1_1;;
    
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &applicationInfo;
    
    // Give our desired layers and extensions to vulkan.
    createInfo.enabledLayerCount = enabledLayers.size();
    createInfo.ppEnabledLayerNames = enabledLayers.data();
    createInfo.enabledExtensionCount = enabledExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    /*
    Actually create the instance.
    Having created the instance, we can actually start using vulkan.
    */
    VK_CHECK_RESULT( vkCreateInstance( &createInfo, NULL, &instance));

    /*
    Register a callback function for the extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME, so that warnings emitted from the validation
    layer are actually printed.
    */
    if (enableValidationLayers) {
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        createInfo.pfnCallback = &debugReportCallbackFn;

        // We have to explicitly load this function.
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

    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        throw std::runtime_error("could not find a device with vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    /*
    Next, we choose a device that can be used for our purposes. 

    With VkPhysicalDeviceFeatures(), we can retrieve a fine-grained list of physical features supported by the device.
    However, in this demo, we are simply launching a simple compute shader, and there are no 
    special physical features demanded for this task.

    With VkPhysicalDeviceProperties(), we can obtain a list of physical device properties. Most importantly,
    we obtain a list of physical device limitations. For this application, we launch a compute shader,
    and the maximum size of the workgroups and total number of compute shader invocations is limited by the physical device,
    and we should ensure that the limitations named maxComputeWorkGroupCount, maxComputeWorkGroupInvocations and 
    maxComputeWorkGroupSize are not exceeded by our application.  Moreover, we are using a storage buffer in the compute shader,
    and we should ensure that it is not larger than the device can handle, by checking the limitation maxStorageBufferRange. 

    However, in our application, the workgroup size and total number of shader invocations is relatively small, and the storage buffer is
    not that large, and thus a vast majority of devices will be able to handle it. This can be verified by looking at some devices at_
    http://vulkan.gpuinfo.org/

    Therefore, to keep things simple and clean, we will not perform any such checks here, and just pick the first physical
    device in the list. But in a real and serious application, those limitations should certainly be taken into account.

    */
    for (VkPhysicalDevice device : devices) {
        if (true) { // As above stated, we do no feature checks, so just accept.
            physicalDevice = device;
            break;
        }
    }
}

// Returns the index of a queue family that supports compute operations. 
uint32_t ComputeApplication::getComputeQueueFamilyIndex() {
    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

    // Retrieve all queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    // Now find a family that supports compute.
    uint32_t currFamilyIndex;
    for (currFamilyIndex = 0; currFamilyIndex < queueFamilies.size(); ++currFamilyIndex) {
        VkQueueFamilyProperties currFamily = queueFamilies[currFamilyIndex];

        if (currFamily.queueCount > 0 && (currFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            // found a queue family with compute. We're done!
            break;
        }
    }

    if (currFamilyIndex == queueFamilies.size()) {
        throw std::runtime_error("could not find a queue family that supports operations");
    }
    return currFamilyIndex;
}

void ComputeApplication::createDevice() {


    //When creating the device, we also specify what queues it has.
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueFamilyIndex = getComputeQueueFamilyIndex(); // find queue family with compute capability.
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1; // get one queue from this family. We don't need more.
    float queuePriorities = 1.0;  // we only have one queue, so this is not that imporant. 
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    //Now we create the logical device. The logical device allows us to interact with the physical device.
    VkDeviceCreateInfo deviceCreateInfo = {};

    // Specify any desired device features here. We do not need any for this application, though.
    VkPhysicalDeviceFeatures deviceFeatures = {};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.enabledLayerCount = enabledLayers.size();  // need to specify validation layers here as well.
    deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; // when creating the logical device, we also specify what queues it has.
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device)); // create logical device.

    uint32_t particularQueueIndex = 0;	//the index within this queue family of the queue to retrieve.

    // Get a handle to the only member of the queue family.
    vkGetDeviceQueue(device, queueFamilyIndex, particularQueueIndex, &computeQueue);
}

// find memory type with desired properties.
uint32_t ComputeApplication::findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    /*
    How does this search work?
    See the documentation of VkPhysicalDeviceMemoryProperties for a detailed description. 
    */
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        //if this memory type's index is set inside memoryTypeBits and it has all the specified properties
        if ((memoryTypeBits & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }
    return -1;
}

void ComputeApplication::createStorageBuffer() {
    /*
    We will now create a buffer. We will render the mandelbrot set into this buffer
    in a computer shade later. 
    */
    
    VkBufferCreateInfo storageBufferCreateInfo = {};
    storageBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    storageBufferCreateInfo.size = storageBufferSize; // buffer size in bytes. 
    storageBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // buffer is used as a storage buffer.
    storageBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // buffer is exclusive to a single queue family at a time. 

    VK_CHECK_RESULT(vkCreateBuffer(device, &storageBufferCreateInfo, NULL, &storageBuffer)); // create buffer.

    /*
    But the buffer doesn't allocate memory for itself, so we must do that manually.
    */

    /*
    First, we find the memory requirements for the buffer.
    */
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, storageBuffer, &memoryRequirements);
    
    /*
    Now use obtained memory requirements info to allocate the memory for the buffer.
    */
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size; // specify required memory.
    /*
    There are several types of memory that can be allocated, and we must choose a memory type that:

    1) Satisfies the memory requirements(memoryRequirements.memoryTypeBits). 
    2) Satifies our own usage requirements. We want to be able to read the buffer memory from the GPU to the CPU
       with vkMapMemory, so we set VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT. 
    Also, by setting VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memory written by the device(GPU) will be easily 
    visible to the host(CPU), without having to call any extra flushing commands. So mainly for convenience, we set
    this flag.
    */
    allocateInfo.memoryTypeIndex = findMemoryType(
        memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, NULL, &storageBufferMemory)); // allocate memory on device.
    
    // Now associate that allocated memory with the buffer. With that, the buffer is backed by actual memory. 
    VK_CHECK_RESULT(vkBindBufferMemory(device, storageBuffer, storageBufferMemory, 0));
}

void ComputeApplication::writeToStorageBuffer(){

    void* mappedMemory;

    vkMapMemory(device, storageBufferMemory, 0, storageBufferSize, 0, &mappedMemory);
    Pixel* pixelPointer = (Pixel*)mappedMemory;

    for (int i = 0; i < OUTPUT_WIDTH * OUTPUT_HEIGHT; i += 1) {
        pixelPointer[i].r = inputImageData[i * 4 + 0];
        pixelPointer[i].g = inputImageData[i * 4 + 1];
        pixelPointer[i].b = inputImageData[i * 4 + 2];
        pixelPointer[i].a = inputImageData[i * 4 + 3];
    }

    // Done reading, so unmap.
    vkUnmapMemory(device, storageBufferMemory);
}
void ComputeApplication::createUniformBuffer(){

    //buffer
    VkBufferCreateInfo uniformBufferCreateInfo = {};
    uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uniformBufferCreateInfo.size = sizeof(UniformBufferObject);
    uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    uniformBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK_RESULT(vkCreateBuffer(device, &uniformBufferCreateInfo, NULL, &uniformBuffer)); // create buffer.

    //bufer memory
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, uniformBuffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(
        memoryRequirements.memoryTypeBits,  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    //allocate memory on device
    VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, NULL, &uniformBufferMemory));

    //bind buffer memory to buffer
    VK_CHECK_RESULT(vkBindBufferMemory(device, uniformBuffer, uniformBufferMemory,0));

}
void ComputeApplication::writeToUniformBuffer(){

    UniformBufferObject ubo;

    ubo.brightness = 1.0f;
    ubo.colorR = 0.0f;
    ubo.colorG = 1.0f;
    ubo.colorB = 0.0f;

    ubo.width = OUTPUT_WIDTH;
    ubo.height = OUTPUT_HEIGHT;

    void* data;

    vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &data);

    memcpy(data, &ubo, sizeof(ubo));

    vkUnmapMemory(device, uniformBufferMemory);

}
void ComputeApplication::createDescriptorSetLayout() {
    /*
    Here we specify a descriptor set layout. This allows us to bind our descriptors to 
    resources in the shader. 

    */

    /*
    Here we specify a binding of type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER to the binding point
    0. This binds to 

      layout(std140, binding = 0) buffer buf

    in the compute shader.
    */
    //define a single binding for the buffer
    VkDescriptorSetLayoutBinding storageBufferBinding = {};
    storageBufferBinding.binding = 0; // binding = 0
    storageBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storageBufferBinding.descriptorCount = 1;
    storageBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    //define a binding for the UBO
    VkDescriptorSetLayoutBinding uniformBufferBinding = {};
    uniformBufferBinding.binding = 1;
    uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferBinding.descriptorCount = 1;
    uniformBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    //put all bindings in an array
    std::array<VkDescriptorSetLayoutBinding, 2> allBindings = {storageBufferBinding, uniformBufferBinding};

    //create descriptor set layout for one binding to a storage buffer
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 2; //number of bindings
    descriptorSetLayoutCreateInfo.pBindings = allBindings.data();

    // Create the descriptor set layout. 
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout));
}

void ComputeApplication::createDescriptorSet() {
    /*
    So we will allocate a descriptor set here.
    But we need to first create a descriptor pool to do that. 
    */

    /*
    Our descriptor pool can only allocate a single storage buffer.
    */
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1; // we only need to allocate one descriptor set from the pool.
    descriptorPoolCreateInfo.poolSizeCount = 2; //2 descriptors total
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

    // create descriptor pool.
    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool));

    /*
    With the pool allocated, we can now allocate the descriptor set. 
    */
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO; 
    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

    // allocate descriptor set.
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

    /*
    Next, we need to connect our actual storage buffer with the descrptor. 
    We use vkUpdateDescriptorSets() to update the descriptor set.
    */

    // Specify the storage buffer to bind to the descriptor.
    VkDescriptorBufferInfo storageBufferInfo = {};
    storageBufferInfo.buffer = storageBuffer;
    storageBufferInfo.offset = 0;
    storageBufferInfo.range = storageBufferSize;

    // Specify the uniform buffer info
    VkDescriptorBufferInfo descriptorUniformBufferInfo = {};
    descriptorUniformBufferInfo.buffer = uniformBuffer;
    descriptorUniformBufferInfo.offset = 0;
    descriptorUniformBufferInfo.range = sizeof(UniformBufferObject);


    std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet; // write to this descriptor set.
    descriptorWrites[0].dstBinding = 0; // write to the first, and only binding.
    descriptorWrites[0].descriptorCount = 1; // update a single descriptor.
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // storage buffer.
    descriptorWrites[0].pBufferInfo = &storageBufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;	//??????
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &descriptorUniformBufferInfo;

    // perform the update of the descriptor set.
    vkUpdateDescriptorSets(device, 2, descriptorWrites.data(), 0, NULL);
}


std::vector<char> ComputeApplication::readFile(const std::string& filename) {

    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
void ComputeApplication::createComputePipeline() {

    
    //Create a shader module. A shader module basically just encapsulates some shader code.
    std::vector<char> shaderCode = readFile("resources/shaders/comp.spv");
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

void ComputeApplication::createCommandBuffer() {
    
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

    
    //Now allocate a command buffer from the command pool. 
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool; // specify the command pool to allocate from. 

    // if the command buffer is primary, it can be directly submitted to queues. 
    // A secondary buffer has to be called from some primary command buffer, and cannot be directly 
    // submitted to a queue. To keep things simple, we use a primary command buffer. 
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1; // allocate a single command buffer. 
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer)); // allocate command buffer.

    /*
    Now we shall start recording commands into the newly allocated command buffer. 
    */
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // the buffer is only submitted and used once in this application.
    VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo)); // start recording commands.

    /*
    We need to bind a pipeline, AND a descriptor set before we dispatch.

    The validation layer will NOT give warnings if you forget these, so be very careful not to forget them.
    */
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

    /*
    Calling vkCmdDispatch basically starts the compute pipeline, and executes the compute shader.
    The number of workgroups is specified in the arguments.
    If you are already familiar with compute shaders from OpenGL, this should be nothing new to you.
    */
    vkCmdDispatch(commandBuffer, (uint32_t)ceil(OUTPUT_WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(OUTPUT_HEIGHT / float(WORKGROUP_SIZE)), 1);

    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer)); // end recording commands.
}

void ComputeApplication::runCommandBuffer() {

    //Now we shall finally submit the recorded command buffer to a the compute queue.
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1; // submit a single command buffer
    submitInfo.pCommandBuffers = &commandBuffer; // the command buffer to submit.

    //Create a fence to make the CPU wait for the GPU to finish before proceeding 
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &fence));

    //We submit the command buffer on the queue, at the same time giving a fence.
    VK_CHECK_RESULT(vkQueueSubmit(computeQueue, 1, &submitInfo, fence));


    /*The command will not have finished executing until the fence is signalled.
    So we wait here.
    We will directly after this read our buffer from the GPU,
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

    //free input image
   	stbi_image_free(inputImageData);
    //free export image
    vkFreeMemory(device, storageBufferMemory, NULL);
    vkDestroyBuffer(device, storageBuffer, NULL);  

    //free uniform buffer
    vkFreeMemory(device, uniformBufferMemory, NULL);
    vkDestroyBuffer(device, uniformBuffer, NULL);


    
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyPipeline(device, computePipeline, NULL);
    vkDestroyCommandPool(device, commandPool, NULL);        
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);              
}

//debug callback
VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFn(
        VkDebugReportFlagsEXT                       flags,
        VkDebugReportObjectTypeEXT                  objectType,
        uint64_t                                    object,
        size_t                                      location,
        int32_t                                     messageCode,
        const char*                                 pLayerPrefix,
        const char*                                 pMessage,
        void*                                       pUserData) 

{

        printf("Debug Report: %s: %s\n", pLayerPrefix, pMessage);

        return VK_FALSE;
}