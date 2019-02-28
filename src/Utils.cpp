#include <Utils.h>
#include <RenderApplication.h>

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#endif

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#endif

#include <map>



///////////////////////////////Utils//////////////////////////////////

void Utils::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
	VkMemoryPropertyFlags propertyFlags, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
	

	//create buffer
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(RenderApplication::device, &createInfo, NULL, &buffer));

	//create buffer memory
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(RenderApplication::device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, propertyFlags);

	VK_CHECK_RESULT(vkAllocateMemory(RenderApplication::device, &allocateInfo, NULL, &bufferMemory));

	VK_CHECK_RESULT(vkBindBufferMemory(RenderApplication::device, buffer, bufferMemory, 0));
}

void Utils::createImage(VkExtent2D dimensions, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
	VkMemoryPropertyFlags propertyFlags, VkImage &image, VkDeviceMemory& imageMemory, bool cubeMapFlag) {
	
	//Texture Image
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = dimensions.width;
	imageInfo.extent.height = dimensions.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	if (cubeMapFlag) {
		imageInfo.arrayLayers = 6;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	VK_CHECK_RESULT(vkCreateImage(RenderApplication::device, &imageInfo, NULL, &image));

	//Texture Image Memory
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(RenderApplication::device, image, &memoryRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, propertyFlags);

	VK_CHECK_RESULT(vkAllocateMemory(RenderApplication::device, &allocInfo, NULL, &imageMemory));

	VK_CHECK_RESULT(vkBindImageMemory(RenderApplication::device, image, imageMemory, 0));
}

void Utils::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size){

	VkCommandBuffer singleTimeCommandBuffer = beginSingleTimeCommandBuffer();

	VkBufferCopy copyInfo = {};
	copyInfo.size = size;

	vkCmdCopyBuffer(singleTimeCommandBuffer, srcBuffer, dstBuffer, 1, &copyInfo);

	endSingleTimeCommandBuffer(singleTimeCommandBuffer);
}

void Utils::createImageView(VkImage image, VkImageView &imageView, VkFormat format, 
	VkImageAspectFlags aspectFlags, bool cubeMapFlag) {

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	if (cubeMapFlag) {
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		imageViewCreateInfo.subresourceRange.layerCount = 6;
	}

	VK_CHECK_RESULT(vkCreateImageView(RenderApplication::device, &imageViewCreateInfo, NULL, &imageView));
}

void Utils::copyBufferToImage(VkBuffer buffer, VkImage image, VkExtent2D imageDimensions, bool cubeMapFlag) {

	VkCommandBuffer singleTimeCommandBuffer = beginSingleTimeCommandBuffer();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	if (cubeMapFlag) {
		region.imageSubresource.layerCount = 6;
	}
	region.imageOffset = { 0,0,0 };
	region.imageExtent = {
		imageDimensions.width,
		imageDimensions.height,
		1
	};

	vkCmdCopyBufferToImage(
		singleTimeCommandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
	endSingleTimeCommandBuffer(singleTimeCommandBuffer);
}

void Utils::copyImageToBuffer(VkBuffer buffer, VkImage image, VkExtent2D imageDimensions){

	VkCommandBuffer singleTimeCommandBuffer = beginSingleTimeCommandBuffer();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0,0,0 };
	region.imageExtent = {
		imageDimensions.width,
		imageDimensions.height,
		1
	};

	vkCmdCopyImageToBuffer(
		singleTimeCommandBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		buffer,
		1,
		&region
	);

	endSingleTimeCommandBuffer(singleTimeCommandBuffer);

}


void Utils::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, bool cubeMapFlag) {

	VkCommandBuffer singleTimeCommandBuffer = beginSingleTimeCommandBuffer();

	VkImageMemoryBarrier barrier = {};

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (cubeMapFlag) {
		barrier.subresourceRange.layerCount = 6;
	}
	VkPipelineStageFlags sourceStage, destinationStage;


	//to allow for copying to
	if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	//to allow shader to read
	else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	//depth buffer
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	

	//no other layout combo options
	else {
		throw std::runtime_error("unsupported layout transition!");
	}


	vkCmdPipelineBarrier(
		singleTimeCommandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommandBuffer(singleTimeCommandBuffer);
}


VkCommandBuffer Utils::beginSingleTimeCommandBuffer() {

	VkCommandBuffer newCommandBuffer;

	//allocate command buffer
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = RenderApplication::getTransferCmdPool();		
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(RenderApplication::device, &allocInfo, &newCommandBuffer));

	//begin command buffer
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK_RESULT(vkBeginCommandBuffer(newCommandBuffer, &beginInfo));
	
	return newCommandBuffer;
}

void Utils::endSingleTimeCommandBuffer(VkCommandBuffer singleTimeCmdBuffer) {

	VK_CHECK_RESULT(vkEndCommandBuffer(singleTimeCmdBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &singleTimeCmdBuffer;

	//NEED A GRAPHICS QUEUE
	vkQueueSubmit(RenderApplication::getTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(RenderApplication::getTransferQueue());
	vkFreeCommandBuffers(RenderApplication::device, RenderApplication::getTransferCmdPool(), 1, &singleTimeCmdBuffer);
}

void Utils::createImageSampler(VkSampler &sampler) {

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VK_CHECK_RESULT(vkCreateSampler(RenderApplication::device, &samplerInfo, NULL, &sampler));
}

VkShaderModule Utils::createShaderModule(const std::string& filename) {

	//read shader code int a char array
	std::vector<char> shaderCode = Utils::readFile(filename);

	//create shader model from shader code
	VkShaderModule shaderModule;

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
	VK_CHECK_RESULT(vkCreateShaderModule(RenderApplication::device, &createInfo, NULL, &shaderModule));

	return shaderModule;

}
std::vector<char> Utils::readFile(const std::string& filename) {

	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	//get file byte size, create a char array of same size
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	//go to beginning of file
	file.seekg(0);

	//copy file into buffer
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

// find memory type with desired properties.
uint32_t Utils::findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {

	VkPhysicalDeviceMemoryProperties memoryProperties;

	vkGetPhysicalDeviceMemoryProperties(RenderApplication::physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {

		//if this memory type's index is set inside memoryTypeBits and it has all the specified properties
		if ((memoryTypeBits & (1 << i)) &&
			((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
			return i;
	}
	throw std::runtime_error("Memory type not found");
}

void Utils::loadModel(std::string modelFilename, std::vector<Vertex> &vertexArray, std::vector<uint32_t> &indexArray) {

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, modelFilename.c_str())) {
		throw std::runtime_error("Failed to load model " + modelFilename);
	}
	cout << "loading model " << modelFilename << endl;

	std::map<Vertex, uint32_t> uniqueVertices = {};
	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};

			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
			
			vertex.normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1 - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = (uint32_t)vertexArray.size();
				indexArray.push_back((uint32_t)vertexArray.size());
				vertexArray.push_back(vertex);
			}
			else {
				indexArray.push_back(uniqueVertices[vertex]);
			}
		}
	}
	cout << "Model " << modelFilename << " loaded with " << vertexArray.size() << " vertices and " << (indexArray.size() / 3) << " triangles" << endl << endl;

}


void Utils::createImageFromFile(const string imageName, VkImage &image, VkDeviceMemory &imageMemory, VkImageLayout finalLayout){

	//Load image from disk
	int numChannels = -1;
	VkExtent2D imageExtent;

	unsigned char* imageData = stbi_load(imageName.c_str(), (int*)&imageExtent.width, (int*)&imageExtent.height, &numChannels, STBI_rgb_alpha);
	if (numChannels == -1) {
        std::string error =  "Render Application::loadImage: failed to load image " + imageName + "\n";
        throw std::runtime_error(error.c_str());
    }

	//debug output
    cout << "loaded " << imageName << endl;
    cout << "Num numChannels: " << numChannels << endl;
    cout << "Width: " << imageExtent.width << endl << "Height: " << imageExtent.height << endl;

	//define byte size of image
	VkDeviceSize imageSize = imageExtent.width * imageExtent.height * 4;

	//Create Staging Buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Utils::createBuffer(
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory
	);

	//copy image data over to staging buffer
	void* mappedStagingBuffer;
	vkMapMemory(RenderApplication::device, stagingBufferMemory, 0, imageSize, 0, &mappedStagingBuffer);
	memcpy(mappedStagingBuffer, imageData, imageSize);
	vkUnmapMemory(RenderApplication::device, stagingBufferMemory);

	//create output image
	Utils::createImage(
		imageExtent,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image,
		imageMemory
	);

	//copy image data from staging buffer to output image
	Utils::transitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Utils::copyBufferToImage(stagingBuffer, image, imageExtent);
	Utils::transitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, finalLayout);

	//clean up staging buffer
	vkDestroyBuffer(RenderApplication::device, stagingBuffer, NULL);
	vkFreeMemory(RenderApplication::device, stagingBufferMemory, NULL);

	//delete image fram main memory
	stbi_image_free(imageData);

}

void Utils::createCubeMapImageFromFile(const std::vector<string> imageNames, VkImage &image, VkDeviceMemory &imageMemory, VkImageLayout finalLayout) {

	unsigned char* imageData[6];
	VkExtent2D imageExtent;

	for (int i = 0; i < 6; ++i) {

		//Load image from disk
		int numChannels = -1;
		
		imageData[i] = stbi_load(imageNames[i].c_str(), (int*)&imageExtent.width, (int*)&imageExtent.height, &numChannels, STBI_rgb_alpha);
		if (numChannels == -1) {
			std::string error = "Render Application::loadImage: failed to load image " + imageNames[i] + "\n";
			throw std::runtime_error(error.c_str());
		}
		//debug output
		cout << "\nloaded " << imageNames[i] << endl;
		cout << "Num numChannels: " << numChannels << endl;
		cout << "Width: " << imageExtent.width << endl << "Height: " << imageExtent.height << endl << endl;
	}

	
	//define byte size of image
	VkDeviceSize faceSize = imageExtent.width * imageExtent.height * 4;
	VkDeviceSize cubeMapSize = faceSize * 6;

	//Create Staging Buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Utils::createBuffer(
		cubeMapSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory
	);

	
	//copy image data over to staging buffer
	void* mappedStagingBuffer;
	vkMapMemory(RenderApplication::device, stagingBufferMemory, 0, cubeMapSize, 0, &mappedStagingBuffer);


	for (int i = 0; i < 6; ++i) {
		memcpy((unsigned char*)mappedStagingBuffer + (faceSize * i), imageData[i], faceSize);
	}
	vkUnmapMemory(RenderApplication::device, stagingBufferMemory);
	
	//create output image
	Utils::createImage(
		imageExtent,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image,
		imageMemory,
		true
	);

	//copy image data from staging buffer to output image
	Utils::transitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, true);
	Utils::copyBufferToImage(stagingBuffer, image, imageExtent, true);
	Utils::transitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, finalLayout, true);

	//clean up staging buffer
	vkDestroyBuffer(RenderApplication::device, stagingBuffer, NULL);
	vkFreeMemory(RenderApplication::device, stagingBufferMemory, NULL);

	//delete all images fram main memory
	for (int i = 0; i < 6; ++i) {
		stbi_image_free(imageData[i]);
	}
	
}

void Utils::exportImageAsPNG(VkImage outputImage, VkExtent2D dimensions, std::string fileName, uint32_t numChannels) {

	//define image byte size
	VkDeviceSize bufferByteSize = dimensions.width * dimensions.height * numChannels;

	//create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Utils::createBuffer(
		bufferByteSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
		stagingBuffer, 
		stagingBufferMemory
	);

	//copy image data from image to buffer
	Utils::copyImageToBuffer(stagingBuffer, outputImage, dimensions);

	//map staging buffer memory and export it
	void* mappedStagingBuffer;
	vkMapMemory(RenderApplication::device, stagingBufferMemory, 0, bufferByteSize, 0, &mappedStagingBuffer);
	stbi_write_png(fileName.c_str(), dimensions.width, dimensions.height, numChannels, mappedStagingBuffer, dimensions.width * numChannels);
	vkUnmapMemory(RenderApplication::device, stagingBufferMemory);


	//Clean Up Staging Buffer
	vkDestroyBuffer(RenderApplication::device, stagingBuffer, nullptr);
	vkFreeMemory(RenderApplication::device, stagingBufferMemory, nullptr);
}
//debug callback
VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFunction(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char*                                 pLayerPrefix,
	const char*                                 pMessage,
	void*                                       pUserData)

{

	printf("\nDebug Report: %s: %s\n\n", pLayerPrefix, pMessage);

	return VK_FALSE;
}