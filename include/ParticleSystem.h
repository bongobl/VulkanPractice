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

	static VkBuffer particleBuffer;
	static VkDeviceMemory particleBufferMemory;

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
	static void createParticleBuffer();
	static void createParticleBufferMemory();

	static void createDescriptorSetLayout();
	static void createDescriptorPool();
	static void createDescriptorSet();


	static void createPhysicsPipeline();
	
	static void createCommandBuffer();
	static void runCommandBuffer();

	friend class RenderApplication;

};