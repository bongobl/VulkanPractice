#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Utils.h>
#include <RenderApplication.h>
class Lighting{

	
	static VkExtent2D shadowMapExtent;
    static VkImage shadowMapDepthImage;
    static VkDeviceMemory shadowMapDepthImageMemory;
	static VkImageView shadowMapDepthImageView;

	static VkBuffer shadowMapUniformBuffer;
	static VkDeviceMemory shadowMapUniformBufferMemory;

	static VkDescriptorSetLayout shadowMapDescriptorSetLayout;
	static VkDescriptorPool shadowMapDescriptorPool;
	static VkDescriptorSet shadowMapDescriptorSet;

	static VkFramebuffer shadowMapFrameBuffer;
	static VkRenderPass shadowMapRenderPass;
	static VkPipeline shadowMapGraphicsPipeline;
	static VkCommandBuffer shadowMapCommandBuffer;

public:
    
    static glm::vec3 direction;
    
    static void createShadowResources();
	static void destroyShadowResources();
	static void writeToShadowMapUniformBuffer();
private:

	static void createShadowMapDepthImage();
	static void createShadowMapDepthImageView();
	static void createShadowMapUniformBuffer();

	static void createShadowMapDescriptorSetLayout();
	static void createShadowMapDescriptorPool();
	static void createShadowMapDescriptorSet();

	static void createShadowMapFrameBuffer();
	static void createShadowMapRenderPass();
	static void createShadowMapGraphicsPipeline();
	static void createShadowMapCommandBuffer();
	


};