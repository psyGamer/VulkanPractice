#pragma once

#include "VulkanUtils.h"

class Vertex {

public:
	glm::vec3 position;
	glm::vec2 uvCoord;
	glm::vec3 color;

public:
	Vertex(glm::vec3 position, glm::vec2 uvCoord);
	Vertex(glm::vec3 position, glm::vec2 uvCoord, glm::vec3 color);

	static VkVertexInputBindingDescription getBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};