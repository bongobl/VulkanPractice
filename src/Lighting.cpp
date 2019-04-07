#include <Lighting.h>

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

VkExtent2D Lighting::shadowMapExtent = { 1920,1470};
VkImage Lighting::shadowMapDepthImage;
VkDeviceMemory Lighting::shadowMapDepthImageMemory;
VkImageView Lighting::shadowMapDepthImageView;

VkBuffer Lighting::shadowMapTessShaderUBO;
VkDeviceMemory Lighting::shadowMapTessShaderUBOMemory;

VkDescriptorSetLayout Lighting::shadowMapDescriptorSetLayout;
VkDescriptorPool Lighting::shadowMapDescriptorPool;
VkDescriptorSet Lighting::shadowMapDescriptorSet;

VkFramebuffer Lighting::shadowMapFrameBuffer;
VkRenderPass Lighting::shadowMapRenderPass;
VkPipelineLayout Lighting::shadowMapPipelineLayout;
VkPipeline Lighting::shadowMapGraphicsPipeline;
VkCommandBuffer Lighting::shadowMapCommandBuffer;
glm::vec3 Lighting::direction = glm::normalize(glm::vec3(2.5f, -2, -3.5f));


void Lighting::createShadowResources(){

	createShadowMapDepthImage();
	createShadowMapDepthImageView();
	createShadowMapTessShaderUBO();

	createShadowMapDescriptorSetLayout();
	createShadowMapDescriptorPool();
	createShadowMapDescriptorSet();

	createShadowMapRenderPass();
	createShadowMapFrameBuffer();
	createShadowMapGraphicsPipeline();
	createShadowMapCommandBuffer();
}

void Lighting::destroyShadowResources(){

	//free shadowmap image
	vkDestroyImageView(RenderApplication::device, shadowMapDepthImageView, NULL);
	vkDestroyImage(RenderApplication::device, shadowMapDepthImage, NULL);
	vkFreeMemory(RenderApplication::device, shadowMapDepthImageMemory, NULL);

	//free uniform buffer
	vkDestroyBuffer(RenderApplication::device, shadowMapTessShaderUBO, NULL);
	vkFreeMemory(RenderApplication::device, shadowMapTessShaderUBOMemory, NULL);

	//free descriptor resources
	vkDestroyDescriptorPool(RenderApplication::device, shadowMapDescriptorPool, NULL);
	vkDestroyDescriptorSetLayout(RenderApplication::device, shadowMapDescriptorSetLayout, NULL);

	vkDestroyFramebuffer(RenderApplication::device, shadowMapFrameBuffer, NULL);
	vkDestroyPipeline(RenderApplication::device, shadowMapGraphicsPipeline, NULL);
	vkDestroyPipelineLayout(RenderApplication::device, shadowMapPipelineLayout, NULL);
	vkDestroyRenderPass(RenderApplication::device, shadowMapRenderPass, NULL);
}

//hardcoded for now
void Lighting::writeToShadowMapTessShaderUBO(glm::mat4 model){

	//Copy over Vertex Shader UBO
    UniformDataTessShader tessShaderData;

	glm::vec3 cameraPosition(0, 7.8f, 8.8f);

	tessShaderData.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.03f, 0.03f, 0.03f));
	tessShaderData.view = glm::lookAt(cameraPosition, glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
	tessShaderData.projection = glm::perspective(glm::radians(45.0f), (float)(SwapChain::extent.width) / SwapChain::extent.height, 0.2f, 300.0f);
	tessShaderData.projection[1][1] *= -1;

	void* mappedMemory;

	vkMapMemory(RenderApplication::device, shadowMapTessShaderUBOMemory, 0, sizeof(tessShaderData), 0, &mappedMemory);
	memcpy(mappedMemory, &tessShaderData, sizeof(tessShaderData));
	vkUnmapMemory(RenderApplication::device, shadowMapTessShaderUBOMemory);
}

//////////////Private Functions////////////////////////////
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

void Lighting::createShadowMapTessShaderUBO(){

	//Create UBO for the tess shader data
	Utils::createBuffer(
		sizeof(UniformDataTessShader),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		shadowMapTessShaderUBO, shadowMapTessShaderUBOMemory
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
    poolSize.descriptorCount = 1;	//we only have one descriptor set right now

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
	tessUBODescriptorWrite.dstSet = shadowMapDescriptorSet;
	tessUBODescriptorWrite.dstBinding = 0;		//binding = 0
	tessUBODescriptorWrite.dstArrayElement = 0;	
		// Descriptor info
		VkDescriptorBufferInfo tessUBODescriptorInfo = {};
		tessUBODescriptorInfo.buffer = shadowMapTessShaderUBO;	
		tessUBODescriptorInfo.offset = 0;
		tessUBODescriptorInfo.range = sizeof(UniformDataTessShader);

	tessUBODescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	tessUBODescriptorWrite.descriptorCount = 1;
	tessUBODescriptorWrite.pBufferInfo = &tessUBODescriptorInfo;

	// perform the update of the descriptor sets
	vkUpdateDescriptorSets(RenderApplication::device, 1, &tessUBODescriptorWrite, 0, NULL);
}

void Lighting::createShadowMapRenderPass() {


	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 0;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;


	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &depthAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;

	VK_CHECK_RESULT(vkCreateRenderPass(RenderApplication::device, &renderPassInfo, nullptr, &shadowMapRenderPass));
}

void Lighting::createShadowMapFrameBuffer() {

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = shadowMapRenderPass;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &shadowMapDepthImageView;
	createInfo.width = shadowMapExtent.width;
	createInfo.height = shadowMapExtent.height;
	createInfo.layers = 1;

	VK_CHECK_RESULT(vkCreateFramebuffer(RenderApplication::device, &createInfo, NULL, &shadowMapFrameBuffer));

}

void Lighting::createShadowMapGraphicsPipeline() {

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
	VkShaderModule fragmentShaderModule = Utils::createShaderModule("resources/shaders/shadowFrag.spv");

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragmentShaderModule;
	fragmentShaderStageInfo.pName = "main";

	
	VkPipelineShaderStageCreateInfo shaderStages[] = { 
		vertexShaderStageInfo, 
		tessContShaderStageInfo, 
		tessEvalShaderStageInfo, 
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
	viewport.width = (float)shadowMapExtent.width;
	viewport.height = (float)shadowMapExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	//Scissors
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = shadowMapExtent;

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

	//Pipeline Layout, need to know about descriptor set layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &shadowMapDescriptorSetLayout;


	//Create Pipeline Layout
	VK_CHECK_RESULT(vkCreatePipelineLayout(RenderApplication::device, &pipelineLayoutInfo, NULL, &shadowMapPipelineLayout));


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
	pipelineInfo.pColorBlendState = NULL; //can't be non-null if using color attachments
	pipelineInfo.pDynamicState = NULL;
	pipelineInfo.layout = shadowMapPipelineLayout;
	pipelineInfo.renderPass = shadowMapRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(RenderApplication::device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &shadowMapGraphicsPipeline));

	//destroy shader modules since we don't need their source code anymore
	vkDestroyShaderModule(RenderApplication::device, vertexShaderModule, NULL);
	vkDestroyShaderModule(RenderApplication::device, fragmentShaderModule, NULL);
	vkDestroyShaderModule(RenderApplication::device, tessContShaderModule, NULL);
	vkDestroyShaderModule(RenderApplication::device, tessEvalShaderModule, NULL);
}

void Lighting::createShadowMapCommandBuffer() {
	
	//Allocate command buffer
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = RenderApplication::getGraphicsCmdPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(RenderApplication::device, &allocInfo, &shadowMapCommandBuffer));

	//begin command buffer
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = NULL;

	//main command buffer scope
	VK_CHECK_RESULT(vkBeginCommandBuffer(shadowMapCommandBuffer, &beginInfo));

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = shadowMapRenderPass;
		renderPassInfo.framebuffer = shadowMapFrameBuffer;
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = shadowMapExtent;

		VkClearValue depthResetValue = {};
		depthResetValue.depthStencil = { 1, 0 };

		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &depthResetValue;

		//render pass scope
		vkCmdBeginRenderPass(shadowMapCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			//bind our graphics pipeline
			vkCmdBindPipeline(shadowMapCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapGraphicsPipeline);

			//bind vertex buffer
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(shadowMapCommandBuffer, 0, 1, &RenderApplication::vertexBuffer, offsets);

			//bind index buffer
			vkCmdBindIndexBuffer(shadowMapCommandBuffer, RenderApplication::indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			//bind descriptor set
			vkCmdBindDescriptorSets(shadowMapCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapPipelineLayout, 0, 1, &shadowMapDescriptorSet, 0, NULL);

			//invoke graphics pipeline and draw
			vkCmdDrawIndexed(shadowMapCommandBuffer, (uint32_t)RenderApplication::indexArray.size(), 1, 0, 0, 0);

		vkCmdEndRenderPass(shadowMapCommandBuffer);

	VK_CHECK_RESULT(vkEndCommandBuffer(shadowMapCommandBuffer));
}