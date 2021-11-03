#pragma once

#include <glm/gtx/hash.hpp>

#include "VulkanUtils.h"

class Vertex {

public:
	glm::vec3 position;
	glm::vec2 uvCoord;
	glm::vec3 color;

public:
	Vertex(glm::vec3 position, glm::vec2 uvCoord);
	Vertex(glm::vec3 position, glm::vec2 uvCoord, glm::vec3 color);

	bool operator==(const Vertex& other) const;

	static VkVertexInputBindingDescription getBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(const Vertex& vertex) const {
			size_t h1 = hash<glm::vec3>()(vertex.position);
			size_t h2 = hash<glm::vec2>()(vertex.uvCoord);
			size_t h3 = hash<glm::vec3>()(vertex.color);

			return ((h1 ^ (h3 << 1)) >> 1) ^ h2;
		}
	};
}