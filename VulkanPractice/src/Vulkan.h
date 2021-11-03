#pragma once

#pragma warning(disable : 26812)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <memory>
#include <string>
#include <vector>

#include <iostream>

// Ensures that condition is NOT true
#define ASSERT(condition, errorMessage)\
	if (!!(condition)) {\
		std::cerr << errorMessage;\
		__debugbreak();\
	}

// Ensures that VkResult is VK_SUCCESS
#define ASSERT_VK(exp)\
	{\
		VkResult __assert_vk_result = (exp);\
		ASSERT(__assert_vk_result != VK_SUCCESS, "An unexpected error occurred on " << __FILE__ << ":" << __LINE__ << "\n\tError Code: " << __assert_vk_result);\
	}
