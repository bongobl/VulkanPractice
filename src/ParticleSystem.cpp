#include <ParticleSystem.h>

std::vector<Vertex> ParticleSystem::particleArray;
VkBuffer ParticleSystem::particleBuffer;
VkDeviceMemory ParticleSystem::particleBufferMemory;

VkDescriptorSetLayout ParticleSystem::descriptorSetLayout;
VkDescriptorPool ParticleSystem::descriptorPool;
VkDescriptorSet ParticleSystem::descriptorSet;

VkPipelineLayout ParticleSystem::physicsPipelineLayout;
VkPipeline ParticleSystem::physicsPipeline;

//command buffer
static VkCommandBuffer commandBuffer;

void ParticleSystem::init(){
	loadParticlesFromModelFile("resources/models/Heptoroid.obj");

	createParticleBuffer();
	createParticleBufferMemory();

	
	createDescriptorSetLayout();
	createDescriptorPool();
	createDescriptorSet();

	createPhysicsPipeline();
	
	createCommandBuffer();
	
}
void ParticleSystem::cleanUp() {
	//free vertex buffer
	vkDestroyBuffer(RenderApplication::device, particleBuffer, NULL);
	vkFreeMemory(RenderApplication::device, particleBufferMemory, NULL);

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


void ParticleSystem::createParticleBuffer() {

	VkDeviceSize particleArraySize = particleArray.size() * sizeof(Vertex);
	Utils::createBuffer(
		particleArraySize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		particleBuffer, particleBufferMemory
	);
}
void ParticleSystem::createParticleBufferMemory() {

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

	//copy contents of our hard coded triangle into the staging buffer
	void* mappedMemory;
	vkMapMemory(RenderApplication::device, stagingBufferMemory, 0, particleArraySize, 0, &mappedMemory);
	memcpy(mappedMemory, particleArray.data(), (size_t)particleArraySize);
	vkUnmapMemory(RenderApplication::device, stagingBufferMemory);

	//copy contents of staging buffer to vertex buffer
	Utils::copyBuffer(stagingBuffer, particleBuffer, particleArraySize);

	//destroy staging buffer
	vkDestroyBuffer(RenderApplication::device, stagingBuffer, NULL);
	vkFreeMemory(RenderApplication::device, stagingBufferMemory, NULL);
}


void ParticleSystem::createDescriptorSetLayout(){
	cout << "called" << endl;
}
void ParticleSystem::createDescriptorPool(){

}
void ParticleSystem::createDescriptorSet(){

}
void ParticleSystem::createPhysicsPipeline(){

}
void ParticleSystem::createCommandBuffer(){

}
void ParticleSystem::runCommandBuffer(){
	
}