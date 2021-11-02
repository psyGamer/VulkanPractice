#include "Vertex.h"

Vertex::Vertex(glm::vec2 position, glm::vec2 uvCoord)
	: Vertex(position, uvCoord, glm::vec3{ 1.0f, 1.0f, 1.0f }) { }
Vertex::Vertex(glm::vec2 position, glm::vec2 uvCoord, glm::vec3 color)
	: position(position), uvCoord(uvCoord), color(color) { }

VkVertexInputBindingDescription Vertex::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, uvCoord);

	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, color);

	return attributeDescriptions;
}