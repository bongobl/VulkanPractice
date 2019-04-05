#include <Lighting.h>

VkExtent2D Lighting::shadowMapExtent = { 1000, 1000};
VkImage Lighting::shadowMapDepthImage;
VkDeviceMemory Lighting::shadowMapDepthImageMemory;
VkImageView Lighting::shadowMapDepthImageView;

VkBuffer Lighting::shadowMapUniformBuffer;
VkDeviceMemory Lighting::shadowMapUniformBufferMemory;

VkDescriptorSetLayout Lighting::shadowMapDescriptorSetLayout;
VkDescriptorPool Lighting::shadowMapDescriptorPool;
VkDescriptorSet Lighting::shadowMapDescriptorSet;

VkFramebuffer Lighting::shadowMapFrameBuffer;
VkRenderPass Lighting::shadowMapRenderPass;
VkPipeline Lighting::shadowMapGraphicsPipeline;
VkCommandBuffer Lighting::shadowMapCommandBuffer;
glm::vec3 Lighting::direction = glm::normalize(glm::vec3(2.5f, -2, -3.5f));


void Lighting::createShadowResources(){

	createShadowMapDepthImage();
	createShadowMapDepthImageView();
	createShadowMapUniformBuffer();

	createShadowMapDescriptorSetLayout();
	createShadowMapDescriptorPool();
	createShadowMapDescriptorSet();

	createShadowMapFrameBuffer();
	createShadowMapRenderPass();
	createShadowMapGraphicsPipeline();
	createShadowMapCommandBuffer();
}

void Lighting::destroyShadowResources(){

	//free shadowmap image
	vkDestroyImageView(RenderApplication::device, shadowMapDepthImageView, NULL);
	vkDestroyImage(RenderApplication::device, shadowMapDepthImage, NULL);
	vkFreeMemory(RenderApplication::device, shadowMapDepthImageMemory, NULL);

	//free uniform buffer
	vkDestroyBuffer(RenderApplication::device, shadowMapUniformBuffer, NULL);
	vkFreeMemory(RenderApplication::device, shadowMapUniformBufferMemory, NULL);

	//free descriptor resources
	vkDestroyDescriptorPool(RenderApplication::device, shadowMapDescriptorPool, NULL);
	vkDestroyDescriptorSetLayout(RenderApplication::device, shadowMapDescriptorSetLayout, NULL);
}

static void writeToShadowMapUniformBuffer(){

}
void Lighting::createShadowMapDepthImage() {

	Utils::createImage(
		shadowMapExtent,
		VK_FORMAT_D32_SFLOAT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		shadowMapDepthImage,
		shadowMapDepthImageMemory
	);

	//we choose to transition layout here (not in render pass) since the transition only needs to happen once in a realtime app
	Utils::transitionImageLayout(shadowMapDepthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Lighting::createShadowMapDepthImageView() {
	
	Utils::createImageView(
		shadowMapDepthImage, 
		shadowMapDepthImageView, 
		VK_FORMAT_D32_SFLOAT, 
		VK_IMAGE_ASPECT_DEPTH_BIT
	);
}

void Lighting::createShadowMapUniformBuffer(){

	//Create UBO for the tess shader data
	Utils::createBuffer(
		sizeof(UniformDataTessShader),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		shadowMapUniformBuffer, shadowMapUniformBufferMemory
	);
}

void Lighting::createShadowMapDescriptorSetLayout() {

	//define a binding for our tesselation eval shader UBO
    VkDescriptorSetLayoutBinding tessShaderUBOBinding = {};
	tessShaderUBOBinding.binding = 0;	//binding = 0
	tessShaderUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	tessShaderUBOBinding.descriptorCount = 1;
	tessShaderUBOBinding.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

	//create descriptor set layout for binding to a UBO
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &tessShaderUBOBinding;

    // Create the descriptor set layout.
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(RenderApplication::device, &layoutInfo, NULL, &shadowMapDescriptorSetLayout));
}

void Lighting::createShadowMapDescriptorPool() {

	VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &poolSize;

    //Create descriptor pool.
    VK_CHECK_RESULT(vkCreateDescriptorPool(RenderApplication::device, &descriptorPoolCreateInfo, NULL, &shadowMapDescriptorPool));
}

void Lighting::createShadowMapDescriptorSet() {

	//With the pool allocated, we can now allocate the descriptor set.
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = shadowMapDescriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &shadowMapDescriptorSetLayout;


    // allocate descriptor set.
    VK_CHECK_RESULT(vkAllocateDescriptorSets(RenderApplication::device, &descriptorSetAllocateInfo, &shadowMapDescriptorSet));

	// Descriptor for our tesselation shader Uniform Buffer
	VkWriteDescriptorSet tessUBODescriptorWrite = {};
	tessUBODescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	tessUBODescriptorWrite.dstSet =shadowMapDescriptorSet;
	tessUBODescriptorWrite.dstBinding = 0;		//binding = 0
	tessUBODescriptorWrite.dstArrayElement = 0;	
		// Descriptor info
		VkDescriptorBufferInfo tessUBODescriptorInfo = {};
		tessUBODescriptorInfo.buffer = shadowMapUniformBuffer;	
		tessUBODescriptorInfo.offset = 0;
		tessUBODescriptorInfo.range = sizeof(UniformDataTessShader);

	tessUBODescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	tessUBODescriptorWrite.descriptorCount = 1;
	tessUBODescriptorWrite.pBufferInfo = &tessUBODescriptorInfo;

	// perform the update of the descriptor sets
	vkUpdateDescriptorSets(RenderApplication::device, 1, &tessUBODescriptorWrite, 0, NULL);
}

void Lighting::createShadowMapFrameBuffer() {

}

void Lighting::createShadowMapRenderPass() {

}

void Lighting::createShadowMapGraphicsPipeline() {

}

void Lighting::createShadowMapCommandBuffer() {

}