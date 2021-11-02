#pragma once

#pragma warning(disable : 26812)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

// Ensures that VkResult is VK_SUCCESS
#define ASSERT_VK(exp)\
	{\
		VkResult __assert_vk_result = (exp);\
		if (__assert_vk_result != VK_SUCCESS) {\
			std::cerr << "An unexpected error occurred on " << __FILE__ << ":" << __LINE__ << "\n\tError Code: " << __assert_vk_result;\
			__debugbreak();\
		}\
	}

// Use at your own risk
#define GET_VK(vkName, varName, ...)\
	Vk ## vkName varName;\
	vkGet ## vkName ## (__VA_ARGS__, &varName)
// Converts a Vulkan version int to a human readable stirng
#define VERION_STR_VK(ver)\
	(std::to_string(VK_API_VERSION_MAJOR(ver))\
		+ std::string(".") + std::to_string(VK_API_VERSION_MINOR(ver))\
		+ std::string(".") + std::to_string(VK_API_VERSION_PATCH(ver)))
// Returns "Yes" or "No" depending on, if the specified bit is set
#define YES_NO_BIT(val, bit) (((val & bit) != 0) ? "Yes" : "No")

std::vector<char> readFile(const std::string & filePath);

void copyBuffer(VkInstance& instance, VkDevice& device, VkQueue& queue, VkCommandPool& commandPool, VkBuffer& src, VkBuffer& dst, VkDeviceSize size);

void createBuffer(VkInstance& instance, VkDevice& device, VkBuffer& buffer, VkDeviceMemory& bufferDeviceMemory, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
template <typename T>
void createAndUploadBuffer(VkInstance& instance, VkDevice& device, VkQueue& queue, VkCommandPool& commandPool, std::vector<T> data, VkBuffer& buffer, VkDeviceMemory& bufferDeviceMemory, VkBufferUsageFlags bufferUsageFlags);

uint32_t findMemoryTypeIndex(VkInstance& instance, uint32_t typeFilter, VkMemoryPropertyFlags properties);
uint32_t getSwapchainImageCount(VkDevice & device, VkSwapchainKHR & swapchain);
std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance & instance);
void createShaderModule(const std::vector<char>&code, VkShaderModule * shaderModule, VkDevice & device);

#ifdef _DEBUG
void printLayerPropertiesInfo();
void printExtensionPropertiesInfo();
void printPhysicalDevicesInfo(VkInstance & instance, VkSurfaceKHR & surface);
#endif

#include "VulkanUtils.tpp"