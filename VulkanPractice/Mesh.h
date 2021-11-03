#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>

#include "Vertex.h"

class Mesh {

private:
	bool m_Loaded = false;

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Incices;

public:
	Mesh();

	void Create(const std::string& filePath);

	const std::vector<Vertex>& GetVertices();
	const std::vector<uint32_t>& GetIndices();
};