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

	static std::vector<Vertex> particleArray;
	static uint32_t pitch;
	//physics buffer
	static VkBuffer physicsBuffer;
	static VkDeviceMemory physicsBufferMemory;

	//vertex buffer
	static VkBuffer vertexBuffer;
	static VkDeviceMemory vertexBufferMemory;


	//uniform buffer
	static VkBuffer uniformBuffer;
	static VkDeviceMemory uniformBufferMemory;

	//descriptors
	static VkDescriptorSetLayout descriptorSetLayout;
	static VkDescriptorPool descriptorPool;
	static VkDescriptorSet descriptorSet;

	//pipeline
	static VkPipelineLayout physicsPipelineLayout;
	static VkPipeline physicsPipeline;

	//command buffer
	static VkCommandBuffer physicsCommandBuffer;

	
	
public:
	static void init();
	static void cleanUp();

	static void runPhysicsCommandBuffer();
	static void writeToUniformBuffer();
private:
	static void loadParticlesFromModelFile(string filename);
	static void createBuffers();
	static void writeToBuffers();

	static void createDescriptorSetLayout();
	static void createDescriptorPool();
	static void createDescriptorSet();


	static void createPhysicsComputePipeline();
	
	static void createPhysicsCommandBuffer();

	friend class RenderApplication;

};