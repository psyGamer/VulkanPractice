#pragma once

#include <vector>

#include "Vertex.h"

std::vector<Vertex> getVerticalQuadVertices();
std::vector<Vertex> getHorizontalQuadVertices();
std::vector<uint32_t> getQuadIndices();
