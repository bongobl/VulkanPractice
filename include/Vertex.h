#pragma once
#include <iostream>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>


using namespace std;

struct Vertex {
	glm::vec3 position;
	float mass;
	glm::vec3 normal;
	float padding0;
	glm::vec2 texCoord;
	glm::vec2 padding1;
	glm::vec3 velocity;
	float padding2;
	
	bool operator<(const Vertex& other) const;
	
	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();
	
};