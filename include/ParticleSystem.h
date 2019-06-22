#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <vulkan/vulkan.h>

#include <Vertex.h>
#include <Utils.h>
#include <RenderApplication.h>
class ParticleSystem{

	static std::vector<Vertex> particleArray;

	//vertex buffer
	static VkBuffer vertexBuffer;
	static VkDeviceMemory vertexBufferMemory;

	//physics buffer
	static VkBuffer physicsBuffer;
	static VkDeviceMemory physicsBufferMemory;

	//descriptors
	static VkDescriptorSetLayout descriptorSetLayout;
	static VkDescriptorPool descriptorPool;
	static VkDescriptorSet descriptorSet;

	//pipeline
	static VkPipelineLayout physicsPipelineLayout;
	static VkPipeline physicsPipeline;

	//command buffer
	static VkCommandBuffer commandBuffer;


public:
	static void init();
	static void cleanUp();

private:
	static void loadParticlesFromModelFile(string filename);
	static void createBuffers();
	static void writeToBuffers();


	static void createDescriptorSetLayout();
	static void createDescriptorPool();
	static void createDescriptorSet();


	static void createPhysicsPipeline();
	
	static void createCommandBuffer();
	static void runCommandBuffer();

	friend class RenderApplication;

};