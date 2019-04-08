#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Utils.h>
#include <RenderApplication.h>


class Lighting{

public:

	static glm::vec3 direction;

	class ShadowMap{

		friend class Lighting;
	public:

		static VkExtent2D extent;
		static VkImage depthImage;
		static VkImageView depthImageView;

	private:

		static VkDeviceMemory depthImageMemory;

		static VkBuffer tessShaderUBO;
		static VkDeviceMemory tessShaderUBOMemory;

		static VkDescriptorSetLayout descriptorSetLayout;
		static VkDescriptorPool descriptorPool;
		static VkDescriptorSet descriptorSet;

		static VkRenderPass renderPass;
		static VkFramebuffer frameBuffer;
		static VkPipelineLayout pipelineLayout;
		static VkPipeline graphicsPipeline;
		static VkCommandBuffer commandBuffer;

	public:
		static void init();
		static void destroy();
		static void writeToTessShaderUBO(glm::mat4 model);	//need to figure out how to get same uniform struct from render app
		static void runCommandBuffer();
		static void exportToDisk();
		
	private:
		static void createDepthImage();
		static void createDepthImageView();
		static void createTessShaderUBO();

		static void createDescriptorSetLayout();
		static void createDescriptorPool();
		static void createDescriptorSet();

		static void createRenderPass();
		static void createFrameBuffer();
		static void createGraphicsPipeline();
		static void createCommandBuffer();
	};
    

};