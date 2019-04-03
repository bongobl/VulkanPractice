#include <Lighting.h>

VkImage Lighting::shadowMapDepthImage;
VkDeviceMemory Lighting::shadowMapDepthImageMemory;
VkImageView Lighting::shadowMapDepthImageView;

VkDescriptorSetLayout Lighting::shadowMapDescriptorSetLayout;
VkDescriptorPool Lighting::shadowMapDescriptorPool;
VkDescriptorSet Lighting::shadowMapDescriptorSet;

VkFramebuffer Lighting::shadowMapFrameBuffer;
VkRenderPass Lighting::shadowMapRenderPass;
VkPipeline Lighting::shadowMapGraphicsPipeline;
VkCommandBuffer Lighting::shadowMapCommandBuffer;
glm::vec3 Lighting::direction = glm::normalize(glm::vec3(2.5f, -2, -3.5f));


void Lighting::createShadowResources(){

	createShadowMapDepthImage();
	createShadowMapDepthImageView();

	createShadowMapDescriptorSetLayout();
	createShadowMapDescriptorPool();
	createShadowMapDescriptorSet();

	createShadowMapFrameBuffer();
	createShadowMapRenderPass();
	createShadowMapGraphicsPipeline();
	createShadowMapCommandBuffer();
}

void Lighting::createShadowMapDepthImage() {

}

void Lighting::createShadowMapDepthImageView() {

}

void Lighting::createShadowMapDescriptorSetLayout() {

}

void Lighting::createShadowMapDescriptorPool() {

}

void Lighting::createShadowMapDescriptorSet() {

}

void Lighting::createShadowMapFrameBuffer() {

}

void Lighting::createShadowMapRenderPass() {

}

void Lighting::createShadowMapGraphicsPipeline() {

}

void Lighting::createShadowMapCommandBuffer() {

}