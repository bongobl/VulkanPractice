#include <Vertex.h>


bool Vertex::operator<(const Vertex& other) const {

	//compare based on position
	if (position.x != other.position.x) {
		return position.x < other.position.x;
	}
	else if (position.y != other.position.y) {
		return position.y < other.position.y;
	}
	else if (position.z != other.position.z) {
		return position.z < other.position.z;
	}

	//compare based on normal
	if (normal.x != other.normal.x) {
		return normal.x < other.normal.x;
	}
	else if (normal.y != other.normal.y) {
		return normal.y < other.normal.y;
	}
	else if (normal.z != other.normal.z) {
		return normal.z < other.normal.z;
	}

	//compare based on texCoord
	if (texCoord.x != other.texCoord.x) {
		return texCoord.x < other.texCoord.x;
	}
	else if (texCoord.y != other.texCoord.y) {
		return texCoord.y < other.texCoord.y;
	}

	//compare based on mass
	if (mass != other.mass) {
		return mass < other.mass;
	}

	//compare based on velocity
	if (velocity.x != other.velocity.x) {
		return velocity.x < other.velocity.x;
	}
	else if (velocity.y != other.velocity.y) {
		return velocity.y < other.velocity.y;
	}
	else if (velocity.z != other.velocity.z) {
		return velocity.z < other.velocity.z;
	}
	return false;
}

VkVertexInputBindingDescription Vertex::getBindingDescription() {

	VkVertexInputBindingDescription bindingDescriptions = {};
	bindingDescriptions.binding = 0;
	bindingDescriptions.stride = sizeof(Vertex);
	bindingDescriptions.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}
std::array<VkVertexInputAttributeDescription, 5> Vertex::getAttributeDescriptions() {

	std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, normal);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Vertex, mass);

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(Vertex, velocity);

	return attributeDescriptions;

}