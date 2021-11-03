#include "MeshHelper.h"

std::vector<Vertex> getVerticalQuadVertices() {
	return {
		Vertex({ -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }), // 0 Top Left
		Vertex({  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }), // 1 Top Right
		Vertex({ -0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }), // 2 Bottom Left
		Vertex({  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }), // 3 Bottom Right
	};
}

std::vector<Vertex> getHorizontalQuadVertices() {
	return {
		Vertex({ -0.5f, 0.0f, -0.5f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }), // 0 Top Left
		Vertex({  0.5f, 0.0f, -0.5f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }), // 1 Top Right
		Vertex({ -0.5f, 0.0f,  0.5f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }), // 2 Bottom Left
		Vertex({  0.5f, 0.0f,  0.5f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }), // 3 Bottom Right
	};				    
}

std::vector<uint32_t> getQuadIndices() {
	return {
		2, 3, 0,
		3, 1, 0
	};
}