#include <ParticleSystem.h>

std::vector<Vertex> ParticleSystem::particleArray;
UniformDataComputeShader ParticleSystem::computeShaderData;
glm::vec4 ParticleSystem::netPosition;

VkBuffer ParticleSystem::physicsBuffer;
VkDeviceMemory ParticleSystem::physicsBufferMemory;
VkBuffer ParticleSystem::vertexBuffer;
VkDeviceMemory ParticleSystem::vertexBufferMemory;
std::vector<VkBuffer> ParticleSystem::uniformBuffers;
std::vector<VkDeviceMemory> ParticleSystem::uniformBufferMemories;
VkBuffer ParticleSystem::nextFrameInfo;
VkDeviceMemory ParticleSystem::nextFrameInfoMemory;


VkDescriptorSetLayout ParticleSystem::descriptorSetLayout;
VkDescriptorPool ParticleSystem::descriptorPool;
std::vector<VkDescriptorSet> ParticleSystem::descriptorSets;

VkPipelineLayout ParticleSystem::physicsPipelineLayout;
VkPipeline ParticleSystem::physicsPipeline;

std::vector<VkCommandBuffer> ParticleSystem::physicsCommandBuffers;

void ParticleSystem::init(size_t numSwapChainImages){
	loadParticlesFromModelFile("resources/models/Heptoroid.obj");

	createBuffers(numSwapChainImages);
	writeToVertexBuffer();
	writeToNextFrameInfo();
	
	createDescriptorSetLayout();
	createDescriptorPool(numSwapChainImages);
	createDescriptorSets(numSwapChainImages);

	createPhysicsComputePipeline();
	
	createPhysicsCommandBuffers(numSwapChainImages);

}

void ParticleSystem::refresh(size_t numSwapChainImages){

	//free command buffers (while keeping command pool)
	vkFreeCommandBuffers(RenderApplication::device, RenderApplication::getComputeCommandPool(), (uint32_t)physicsCommandBuffers.size(), physicsCommandBuffers.data());

	//destroy descriptors
	vkDestroyDescriptorPool(RenderApplication::device, descriptorPool, nullptr);

	//free uniform buffers
	for (size_t i = 0; i < uniformBuffers.size(); i++) {
		vkDestroyBuffer(RenderApplication::device, uniformBuffers[i], nullptr);
		vkFreeMemory(RenderApplication::device, uniformBufferMemories[i], nullptr);
	}

	
	//create uniform buffers
	uniformBuffers.resize(numSwapChainImages);
	uniformBufferMemories.resize(numSwapChainImages);
	
	for (unsigned int i = 0; i < uniformBuffers.size(); ++i) {
		Utils::createBuffer(
			sizeof(UniformDataComputeShader),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			uniformBuffers[i], uniformBufferMemories[i]
		);
	}

	createDescriptorPool(numSwapChainImages);
	createDescriptorSets(numSwapChainImages);
	createPhysicsCommandBuffers(numSwapChainImages);
	
}
void ParticleSystem::cleanUp() {

	
	//free physics buffer
	vkDestroyBuffer(RenderApplication::device, physicsBuffer, NULL);
	vkFreeMemory(RenderApplication::device, physicsBufferMemory, NULL);
	
	//free vertex buffer
	vkDestroyBuffer(RenderApplication::device, vertexBuffer, NULL);
	vkFreeMemory(RenderApplication::device, vertexBufferMemory, NULL);
	
	for (unsigned int i = 0; i < uniformBuffers.size(); ++i) {
		//free uniform buffer
		vkDestroyBuffer(RenderApplication::device, uniformBuffers[i], NULL);
		vkFreeMemory(RenderApplication::device, uniformBufferMemories[i], NULL);
	}
	
	//free nextFrameInfo
	vkDestroyBuffer(RenderApplication::device, nextFrameInfo, NULL);
	vkFreeMemory(RenderApplication::device, nextFrameInfoMemory, NULL);

	//free pipeline and layout
	vkDestroyPipeline(RenderApplication::device, physicsPipeline, NULL);
	vkDestroyPipelineLayout(RenderApplication::device, physicsPipelineLayout, NULL);

	//descriptors
	vkDestroyDescriptorPool(RenderApplication::device, descriptorPool, NULL);
	vkDestroyDescriptorSetLayout(RenderApplication::device, descriptorSetLayout, NULL);
}

//Private Helpers

void ParticleSystem::loadParticlesFromModelFile(string filename) {

	std::vector<uint32_t> dummyIndexArray;
	Utils::loadModel(filename, particleArray, dummyIndexArray, true);

	computeShaderData.netMass = 0;
	for (int i = 0; i < particleArray.size(); ++i) {

		//offset position by random value
		glm::vec3 randOffset(Utils::getRandomFloat(-35, 35), Utils::getRandomFloat(-35, 35), Utils::getRandomFloat(-35, 35));
		particleArray[i].position += randOffset;

		//storing random color in vertex's normal attribute
		glm::vec3 randColor(Utils::getRandomFloat(0, 1), Utils::getRandomFloat(0, 1), Utils::getRandomFloat(0, 1));
		particleArray[i].normal = randColor;

		//give particle a random mass
		particleArray[i].mass = Utils::getRandomFloat(3, 40);
		computeShaderData.netMass += particleArray[i].mass;

		//add to running total
		netPosition += glm::vec4(particleArray[i].position * particleArray[i].mass, 0);
	}

	netPosition = netPosition / computeShaderData.netMass;

	computeShaderData.numParticles = (uint32_t)particleArray.size();
	computeShaderData.pitch = (uint32_t)ceil(sqrt(particleArray.size()));
}


void ParticleSystem::createBuffers(size_t numSwapChainImages) {
	
	VkDeviceSize particleArraySize = particleArray.size() * sizeof(Vertex);

	//physics buffer
	Utils::createBuffer(
		particleArraySize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		physicsBuffer, physicsBufferMemory
	);

	//vertex buffer
	Utils::createBuffer(
		particleArraySize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer, vertexBufferMemory
	);

	//uniform buffers
	uniformBuffers.resize(numSwapChainImages);
	uniformBufferMemories.resize(numSwapChainImages);
	
	for (unsigned int i = 0; i < uniformBuffers.size(); ++i) {
		Utils::createBuffer(
			sizeof(UniformDataComputeShader),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			uniformBuffers[i], uniformBufferMemories[i]
		);
	}

	//nextFrameInfo
	//physics buffer
	Utils::createBuffer(
		sizeof(glm::vec4),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		nextFrameInfo, nextFrameInfoMemory
	);

}
void ParticleSystem::writeToVertexBuffer() {

	VkDeviceSize particleArraySize = particleArray.size() * sizeof(Vertex);

	//create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Utils::createBuffer(
		particleArraySize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		stagingBuffer, stagingBufferMemory
	);
	
	//copy contents of our particle array into the staging buffer
	void* mappedMemory;
	vkMapMemory(RenderApplication::device, stagingBufferMemory, 0, particleArraySize, 0, &mappedMemory);
	memcpy(mappedMemory, particleArray.data(), (size_t)particleArraySize);
	vkUnmapMemory(RenderApplication::device, stagingBufferMemory);


	//copy contents of staging buffer to vertex buffer 
	Utils::copyBuffer(stagingBuffer, vertexBuffer, particleArraySize);
	
	
	//destroy staging buffer
	vkDestroyBuffer(RenderApplication::device, stagingBuffer, NULL);
	vkFreeMemory(RenderApplication::device, stagingBufferMemory, NULL);
	
}

void ParticleSystem::writeToNextFrameInfo() {


	//create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Utils::createBuffer(
		sizeof(glm::vec4),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		stagingBuffer, stagingBufferMemory
	);

	//copy contents of our particle array into the staging buffer
	void* mappedMemory;
	vkMapMemory(RenderApplication::device, stagingBufferMemory, 0, sizeof(glm::vec4), 0, &mappedMemory);
	memcpy(mappedMemory, &netPosition, (size_t)sizeof(glm::vec4));
	vkUnmapMemory(RenderApplication::device, stagingBufferMemory);


	//copy contents of staging buffer to vertex buffer 
	Utils::copyBuffer(stagingBuffer, nextFrameInfo, sizeof(glm::vec4));


	//destroy staging buffer
	vkDestroyBuffer(RenderApplication::device, stagingBuffer, NULL);
	vkFreeMemory(RenderApplication::device, stagingBufferMemory, NULL);
}
void ParticleSystem::writeToUniformBuffer(uint32_t imageIndex) {

	computeShaderData.deltaTime = RenderApplication::deltaTime;


	void* mappedMemory;
	vkMapMemory(RenderApplication::device, uniformBufferMemories[imageIndex], 0, sizeof(computeShaderData), 0, &mappedMemory);
	memcpy(mappedMemory, &computeShaderData, sizeof(computeShaderData));
	vkUnmapMemory(RenderApplication::device, uniformBufferMemories[imageIndex]);
}

void ParticleSystem::createDescriptorSetLayout(){
	
	//define a binding for the physics buffer
	VkDescriptorSetLayoutBinding physicsBufferBinding = {};
	physicsBufferBinding.binding = 0;	//binding = 0
	physicsBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	physicsBufferBinding.descriptorCount = 1;
	physicsBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	//define a binding for the vertex buffer
	VkDescriptorSetLayoutBinding vertexBufferBinding = {};
	vertexBufferBinding.binding = 1;	//binding = 1
	vertexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	vertexBufferBinding.descriptorCount = 1;
	vertexBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	
	//define a binding for the uniform buffer
	VkDescriptorSetLayoutBinding uniformBufferBinding = {};
	uniformBufferBinding.binding = 2;	//binding = 2
	uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferBinding.descriptorCount = 1;
	uniformBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	//put all bindings in an array
	std::array<VkDescriptorSetLayoutBinding, 3> allBindings = {
		physicsBufferBinding,
		vertexBufferBinding,
		uniformBufferBinding
	};

	//create descriptor set layout for all above binding points
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t)allBindings.size(); //number of bindings
	layoutInfo.pBindings = allBindings.data();

	// Create the descriptor set layout.
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(RenderApplication::device, &layoutInfo, NULL, &descriptorSetLayout));

}
void ParticleSystem::createDescriptorPool(size_t numSwapChainImages){

	std::array<VkDescriptorPoolSize, 3> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[0].descriptorCount = (uint32_t)numSwapChainImages;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = (uint32_t)numSwapChainImages;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[2].descriptorCount = (uint32_t)numSwapChainImages;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = (uint32_t)numSwapChainImages;
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

	//Create descriptor pool.
	VK_CHECK_RESULT(vkCreateDescriptorPool(RenderApplication::device, &descriptorPoolCreateInfo, NULL, &descriptorPool));
}
void ParticleSystem::createDescriptorSets(size_t numSwapChainImages){

	descriptorSets.resize(numSwapChainImages);

	//All descriptor sets use same layout, we need one descriptor set per swapchain image
	std::vector<VkDescriptorSetLayout> allLayouts(numSwapChainImages, descriptorSetLayout);


	//With the pool allocated, we can now allocate the descriptor set.
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
	descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)descriptorSets.size();
	descriptorSetAllocateInfo.pSetLayouts = allLayouts.data();


	// allocate descriptor set.
	VK_CHECK_RESULT(vkAllocateDescriptorSets(RenderApplication::device, &descriptorSetAllocateInfo, descriptorSets.data()));

	for (unsigned int i = 0; i < descriptorSets.size(); ++i) {

		// Descriptor for our vertex shader Uniform Buffer
		VkWriteDescriptorSet physicsBufferWrite = {};
		physicsBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		physicsBufferWrite.dstSet = descriptorSets[i];
		physicsBufferWrite.dstBinding = 0;		//binding = 0
		physicsBufferWrite.dstArrayElement = 0;
			// Descriptor info
			VkDescriptorBufferInfo physicsBufferInfo = {};
			physicsBufferInfo.buffer = physicsBuffer;
			physicsBufferInfo.offset = 0;
			physicsBufferInfo.range = particleArray.size() * sizeof(Vertex);

		physicsBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		physicsBufferWrite.descriptorCount = 1;
		physicsBufferWrite.pBufferInfo = &physicsBufferInfo;


		// Descriptor for our vertex shader Uniform Buffer
		VkWriteDescriptorSet vertexBufferWrite = {};
		vertexBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vertexBufferWrite.dstSet = descriptorSets[i];
		vertexBufferWrite.dstBinding = 1;		//binding = 1
		vertexBufferWrite.dstArrayElement = 0;
			// Descriptor info
			VkDescriptorBufferInfo vertexBufferInfo = {};
			vertexBufferInfo.buffer = vertexBuffer;
			vertexBufferInfo.offset = 0;
			vertexBufferInfo.range = particleArray.size() * sizeof(Vertex);

		vertexBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vertexBufferWrite.descriptorCount = 1;
		vertexBufferWrite.pBufferInfo = &vertexBufferInfo;

		VkWriteDescriptorSet uniformBufferWrite = {};
		uniformBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uniformBufferWrite.dstSet = descriptorSets[i];
		uniformBufferWrite.dstBinding = 2;		//binding = 2
		uniformBufferWrite.dstArrayElement = 0;
			//Descriptor info
			VkDescriptorBufferInfo uniformBufferInfo = {};
			uniformBufferInfo.buffer = uniformBuffers[i];
			uniformBufferInfo.offset = 0;
			uniformBufferInfo.range = sizeof(UniformDataComputeShader);

		uniformBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferWrite.descriptorCount = 1;
		uniformBufferWrite.pBufferInfo = &uniformBufferInfo;


		//put all descriptor write info in an array
		std::array<VkWriteDescriptorSet, 3> descriptorWrites = {
			physicsBufferWrite,
			vertexBufferWrite,
			uniformBufferWrite
		};

		// perform the update of the descriptor sets
		vkUpdateDescriptorSets(RenderApplication::device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, NULL);

	}//End for
}
void ParticleSystem::createPhysicsComputePipeline(){

	//Compute Shader Stage
	VkShaderModule computeShaderModule = Utils::createShaderModule("resources/shaders/comp.spv");

	VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
	computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	computeShaderStageInfo.module = computeShaderModule;
	computeShaderStageInfo.pName = "main";

	//Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

	VK_CHECK_RESULT(vkCreatePipelineLayout(RenderApplication::device, &pipelineLayoutCreateInfo, NULL, &physicsPipelineLayout));

	//Compute Pipeline
	VkComputePipelineCreateInfo computePipelineInfo = {};
	computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineInfo.stage = computeShaderStageInfo;
	computePipelineInfo.layout = physicsPipelineLayout;

	VK_CHECK_RESULT(vkCreateComputePipelines(RenderApplication::device, VK_NULL_HANDLE, 1, &computePipelineInfo, NULL, &physicsPipeline));

	//destroy shader module
	vkDestroyShaderModule(RenderApplication::device, computeShaderModule, NULL);
}
void ParticleSystem::createPhysicsCommandBuffers(size_t numSwapChainImages){

	//Note: RenderApplication will supply us with an already created command pool for compute operations

	//we want each of our command buffers to draw to a swapchain image
	physicsCommandBuffers.resize(numSwapChainImages);


	//Allocate command buffer
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = RenderApplication::getComputeCommandPool(); // specify the command pool to allocate from.
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = (uint32_t)physicsCommandBuffers.size();
	VK_CHECK_RESULT(vkAllocateCommandBuffers(RenderApplication::device, &commandBufferAllocateInfo, physicsCommandBuffers.data())); // allocate command buffer.

	for (unsigned int i = 0; i < physicsCommandBuffers.size(); ++i) {
		//Begin Info
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		VK_CHECK_RESULT(vkBeginCommandBuffer(physicsCommandBuffers[i], &beginInfo)); // start recording commands.

			//Copy from vertex buffer to physics buffer
			VkBufferCopy copyInfo = {};
			copyInfo.size = particleArray.size() * sizeof(Vertex);
			vkCmdCopyBuffer(physicsCommandBuffers[i], vertexBuffer, physicsBuffer, 1, &copyInfo);

			//Bind Compute Pipeline
			vkCmdBindPipeline(physicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, physicsPipeline);

			//Bind Descriptor Set
			vkCmdBindDescriptorSets(physicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, physicsPipelineLayout, 0, 1, &descriptorSets[i], 0, NULL);

			uint32_t dimensionWorkGroups = (uint32_t)ceil(computeShaderData.pitch / (float)WORKGROUP_SIZE);

			//run compute app
			vkCmdDispatch(physicsCommandBuffers[i], dimensionWorkGroups, dimensionWorkGroups, 1);

		VK_CHECK_RESULT(vkEndCommandBuffer(physicsCommandBuffers[i])); // end recording commands.
	}//End for
}
void ParticleSystem::runPhysicsCommandBuffer(uint32_t imageIndex){
	
	
	//Submit Info
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1; // submit a single command buffer
	submitInfo.pCommandBuffers = &physicsCommandBuffers[imageIndex];

	//Testing only: create a femce
	VkFence fence;
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;
	VK_CHECK_RESULT(vkCreateFence(RenderApplication::device, &fenceCreateInfo, NULL, &fence));

	//Submit fence
	VK_CHECK_RESULT(vkQueueSubmit(RenderApplication::getComputeQueue(), 1, &submitInfo, fence));

	//wait for the fence
	VK_CHECK_RESULT(vkWaitForFences(RenderApplication::device, 1, &fence, VK_TRUE, 100000000000));

	//no longer need fence
	vkDestroyFence(RenderApplication::device, fence, NULL);

}