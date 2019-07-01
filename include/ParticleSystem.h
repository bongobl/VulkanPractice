#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include <cmath>
#include <Vertex.h>
#include <Utils.h>
#include <RenderApplication.h>

static int WORKGROUP_SIZE = 32;
class ParticleSystem{

	//cpu side data
	static std::vector<Vertex> particleArray;
	static UniformDataComputeShader computeShaderData;
	static glm::vec4 netPosition;

	//physics buffer
	static VkBuffer physicsBuffer;
	static VkDeviceMemory physicsBufferMemory;

	//vertex buffer
	static VkBuffer vertexBuffer;
	static VkDeviceMemory vertexBufferMemory;


	//uniform buffer
	static std::vector<VkBuffer> uniformBuffers;
	static std::vector<VkDeviceMemory> uniformBufferMemories;

	//nextFrameData
	static VkBuffer nextFrameInfo;
	static VkDeviceMemory nextFrameInfoMemory;


	//descriptors
	static VkDescriptorSetLayout descriptorSetLayout;
	static VkDescriptorPool descriptorPool;
	static std::vector<VkDescriptorSet> descriptorSets;

	//pipeline
	static VkPipelineLayout physicsPipelineLayout;
	static VkPipeline physicsPipeline;


	
public:

	//command buffer
	static std::vector<VkCommandBuffer> physicsCommandBuffers;

	static void init(size_t numSwapChainImages);
	static void refresh(size_t numSwapChainImages);
	static void cleanUp();

	static void runPhysicsCommandBuffer(uint32_t imageIndex);
	static void writeToUniformBuffer(uint32_t imageIndex);
private:
	static void loadParticlesFromModelFile(string filename);
	static void createBuffers(size_t numSwapChainImages);
	static void writeToVertexBuffer();
	static void writeToNextFrameInfo();

	static void createDescriptorSetLayout();
	static void createDescriptorPool(size_t numSwapChainImages);
	static void createDescriptorSets(size_t numSwapChainImages);


	static void createPhysicsComputePipeline();
	
	static void createPhysicsCommandBuffers(size_t numSwapChainImages);

	friend class RenderApplication;

};