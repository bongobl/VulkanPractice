#pragma once
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Utils.h>
#include <RenderApplication.h>


class Lighting{

public:

	static const glm::vec3 direction;

	class ShadowMap{

	public:

		static glm::mat4 viewMatrix;
		static glm::mat4 projMatrix;

		static VkExtent2D extent;
		static std::vector<VkImage> depthImages;
		static std::vector<VkImageView> depthImageViews;
		static std::vector<VkCommandBuffer> commandBuffers;

	private:

		static size_t numDepthImages;
		static std::vector<VkDeviceMemory> depthImageMemories;

		static std::vector<VkBuffer> tessShaderUBOs;
		static std::vector<VkDeviceMemory> tessShaderUBOMemories;

		static VkDescriptorSetLayout descriptorSetLayout;
		static VkDescriptorPool descriptorPool;	
		static std::vector<VkDescriptorSet> descriptorSets;

		static VkRenderPass renderPass;
		static std::vector<VkFramebuffer> frameBuffers;
		static VkPipelineLayout pipelineLayout;
		static VkPipeline graphicsPipeline;

		

	public:
		static void init(size_t numSwapChainImages);
		static void destroy();
		static void writeToTessShaderUBO(uint32_t imageIndex, glm::mat4 model, glm::mat3 lightOrientation);
		static void runCommandBuffer(uint32_t imageIndex);
		static void exportToDisk(uint32_t imageIndex);
		
	private:
		static void createDepthImages();
		static void createDepthImageViews();
		static void createTessShaderUBOs();

		static void createDescriptorSetLayout();
		static void createDescriptorPool();
		static void createDescriptorSets();

		static void createRenderPass();
		static void createFrameBuffers();
		static void createGraphicsPipeline();
		static void createCommandBuffers();
	};
    

};