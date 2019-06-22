#include <ParticleSystem.h>

std::vector<Vertex> ParticleSystem::particleArray;
VkBuffer ParticleSystem::vertexBuffer;
VkDeviceMemory ParticleSystem::vertexBufferMemory;
VkBuffer ParticleSystem::physicsBuffer;
VkDeviceMemory ParticleSystem::physicsBufferMemory;
VkDescriptorSetLayout ParticleSystem::descriptorSetLayout;
VkDescriptorPool ParticleSystem::descriptorPool;
VkDescriptorSet ParticleSystem::descriptorSet;

VkPipelineLayout ParticleSystem::physicsPipelineLayout;
VkPipeline ParticleSystem::physicsPipeline;

//command buffer
static VkCommandBuffer commandBuffer;

void ParticleSystem::init(){
	loadParticlesFromModelFile("resources/models/Heptoroid.obj");

	createBuffers();
	writeToBuffers();

	
	createDescriptorSetLayout();
	createDescriptorPool();
	createDescriptorSet();

	createPhysicsPipeline();
	
	createCommandBuffer();
	
}
void ParticleSystem::cleanUp() {

	//free vertex buffer
	vkDestroyBuffer(RenderApplication::device, vertexBuffer, NULL);
	vkFreeMemory(RenderApplication::device, vertexBufferMemory, NULL);

	//free physics buffer
	vkDestroyBuffer(RenderApplication::device, physicsBuffer, NULL);
	vkFreeMemory(RenderApplication::device, physicsBufferMemory, NULL);

	//descriptors
	vkDestroyDescriptorPool(RenderApplication::device, descriptorPool, NULL);
	vkDestroyDescriptorSetLayout(RenderApplication::device, descriptorSetLayout, NULL);
}

//Private Helpers

void ParticleSystem::loadParticlesFromModelFile(string filename) {

	std::vector<uint32_t> dummyIndexArray;
	Utils::loadModel(filename, particleArray, dummyIndexArray, true);

	for (int i = 0; i < particleArray.size(); ++i) {
		//storing random color in vertex's normal attribute
		glm::vec3 randColor(Utils::getRandomFloat(0, 1), Utils::getRandomFloat(0, 1), Utils::getRandomFloat(0, 1));
		particleArray[i].normal = randColor;

		//give particle a random mass
		particleArray[i].mass = Utils::getRandomFloat(0, 50);
	}
}


void ParticleSystem::createBuffers() {

	VkDeviceSize particleArraySize = particleArray.size() * sizeof(Vertex);

	//vertex buffer
	Utils::createBuffer(
		particleArraySize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer, vertexBufferMemory
	);

	//physics buffer
	Utils::createBuffer(
		particleArraySize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		physicsBuffer, physicsBufferMemory
	);
}
void ParticleSystem::writeToBuffers() {

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

	//copy contents of staging buffer to physics buffer
	Utils::copyBuffer(stagingBuffer, physicsBuffer, particleArraySize);

	//destroy staging buffer
	vkDestroyBuffer(RenderApplication::device, stagingBuffer, NULL);
	vkFreeMemory(RenderApplication::device, stagingBufferMemory, NULL);

}


void ParticleSystem::createDescriptorSetLayout(){

	//define a binding for the physics buffer
	VkDescriptorSetLayoutBinding physicsBufferBinding = {};
	physicsBufferBinding.binding = 0;	//binding = 0
	physicsBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	physicsBufferBinding.descriptorCount = 1;
	physicsBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	//define a binding for the physics buffer
	VkDescriptorSetLayoutBinding vertexBufferBinding = {};
	vertexBufferBinding.binding = 1;	//binding = 1
	vertexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	vertexBufferBinding.descriptorCount = 1;
	vertexBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	//put all bindings in an array
	std::array<VkDescriptorSetLayoutBinding, 2> allBindings = {
		physicsBufferBinding,
		vertexBufferBinding
	};

	//create descriptor set layout for all above binding points
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t)allBindings.size(); //number of bindings
	layoutInfo.pBindings = allBindings.data();

	// Create the descriptor set layout.
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(RenderApplication::device, &layoutInfo, NULL, &descriptorSetLayout));

}
void ParticleSystem::createDescriptorPool(){

	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

	//Create descriptor pool.
	VK_CHECK_RESULT(vkCreateDescriptorPool(RenderApplication::device, &descriptorPoolCreateInfo, NULL, &descriptorPool));
}
void ParticleSystem::createDescriptorSet(){

	//With the pool allocated, we can now allocate the descriptor set.
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;


	// allocate descriptor set.
	VK_CHECK_RESULT(vkAllocateDescriptorSets(RenderApplication::device, &descriptorSetAllocateInfo, &descriptorSet));

	// Descriptor for our vertex shader Uniform Buffer
	VkWriteDescriptorSet physicsBufferWrite = {};
	physicsBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	physicsBufferWrite.dstSet = descriptorSet;
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
	vertexBufferWrite.dstSet = descriptorSet;
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

	//put all descriptor write info in an array
	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {
		physicsBufferWrite,
		vertexBufferWrite
	};

	// perform the update of the descriptor sets
	vkUpdateDescriptorSets(RenderApplication::device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, NULL);

}
void ParticleSystem::createPhysicsPipeline(){

}
void ParticleSystem::createCommandBuffer(){

}
void ParticleSystem::runCommandBuffer(){
	
}