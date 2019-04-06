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

	static VkBuffer shadowMapTessShaderUBO;
	static VkDeviceMemory shadowMapTessShaderUBOMemory;

	static VkDescriptorSetLayout shadowMapDescriptorSetLayout;
	static VkDescriptorPool shadowMapDescriptorPool;
	static VkDescriptorSet shadowMapDescriptorSet;

	static VkRenderPass shadowMapRenderPass;
	static VkFramebuffer shadowMapFrameBuffer;	
	static VkPipelineLayout shadowMapPipelineLayout;
	static VkPipeline shadowMapGraphicsPipeline;
	static VkCommandBuffer shadowMapCommandBuffer;

public:
    
    static glm::vec3 direction;
    
    static void createShadowResources();
	static void destroyShadowResources();
	static void writeToShadowMapTessShaderUBO(glm::mat4 model);	//need to figure out how to get same uniform struct from render app
private:

	static void createShadowMapDepthImage();
	static void createShadowMapDepthImageView();
	static void createShadowMapTessShaderUBO();

	static void createShadowMapDescriptorSetLayout();
	static void createShadowMapDescriptorPool();
	static void createShadowMapDescriptorSet();

	static void createShadowMapRenderPass();
	static void createShadowMapFrameBuffer();
	static void createShadowMapGraphicsPipeline();
	static void createShadowMapCommandBuffer();
	

};