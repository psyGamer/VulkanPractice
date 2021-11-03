#include "Mesh.h"

#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define CHECK_IF_LOADED\
	if (!m_Loaded) {\
		std::cerr << "This mesh isn't loaded yet!";\
		throw std::logic_error("This mesh isn't loaded yet!");\
	}

Mesh::Mesh() { }

void Mesh::Create(const std::string& filePath) {
	tinyobj::attrib_t vertexAttributes;

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warningString;
	std::string errorString;

	bool success = tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &warningString, &errorString, filePath.c_str());

	if (!success) {
		std::cerr << errorString;
		throw std::runtime_error(errorString);
	}

	if (warningString.length() > 0) {
		std::cout << "[WARING] " << warningString << std::endl;
	}

	std::unordered_map<Vertex, uint32_t> vertexIndices;

	for (auto shape : shapes) {
		for (auto index : shape.mesh.indices) {
			glm::vec3 position = {
				 vertexAttributes.vertices[3 * index.vertex_index + 0],
				-vertexAttributes.vertices[3 * index.vertex_index + 1],
				 vertexAttributes.vertices[3 * index.vertex_index + 2]
			};
			glm::vec3 normals = {
				 vertexAttributes.normals[3 * index.vertex_index + 0],
				-vertexAttributes.normals[3 * index.vertex_index + 1],
				 vertexAttributes.normals[3 * index.vertex_index + 2]
			};

			Vertex vertex(position, glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), normals);

			if (vertexIndices.count(vertex) == 0) {
				vertexIndices[vertex] = vertexIndices.size();
				m_Vertices.push_back(vertex);
			}

			m_Incices.push_back(vertexIndices[vertex]);
		}
	}

	m_Loaded = true;
}

const std::vector<Vertex>& Mesh::GetVertices() {
	CHECK_IF_LOADED

	return m_Vertices;
}
const std::vector<uint32_t>& Mesh::GetIndices() {
	CHECK_IF_LOADED

	return m_Incices;
}

#undef CHECK_IF_LOADED