#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Utils.h>

class Lighting{

    static VkImage shadowMapDepthImage;
    static VkDeviceMemory shadowMapDepthImageMemory;
	static VkImageView shadowMapDepthImageView;

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

private:

	static void createShadowMapDepthImage();
	static void createShadowMapDepthImageView();

	static void createShadowMapDescriptorSetLayout();
	static void createShadowMapDescriptorPool();
	static void createShadowMapDescriptorSet();

	static void createShadowMapFrameBuffer();
	static void createShadowMapRenderPass();
	static void createShadowMapGraphicsPipeline();
	static void createShadowMapCommandBuffer();
	


};