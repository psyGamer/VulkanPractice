#pragma once

#include <vector>
#include <string>

#include "Vulkan.h"

bool isFormatSupported(VkPhysicalDevice& physicalDevice, VkFormat format, VkImageTiling imageTiling, VkFormatFeatureFlags featureFlags);
VkFormat findSupportedFormat(VkPhysicalDevice& physicalDevice, const std::vector<VkFormat>& formats, VkImageTiling imageTiling, VkFormatFeatureFlags featureFlags);
bool isStencilFormat(VkFormat format);

VkCommandBuffer startSingleTimeCommandBuffer(VkDevice& device, VkCommandPool& commandPool);
void endSingleTimeCommandBuffer(VkDevice& device, VkCommandBuffer& commandBuffer, VkQueue& queue, VkCommandPool& commandPool);

void createImage(VkDevice& device, VkPhysicalDevice& physicalDevice, uint32_t width, uint32_t height, VkFormat imageFormat, VkImageTiling imageTiling, VkImageUsageFlags imageUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkImage& image, VkDeviceMemory& imageMemory);
void createImageView(VkDevice& device, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView);

void changeImageLayout(VkDevice& device, VkImage& image, VkFormat format, VkCommandPool& commandPool, VkQueue& queue, VkImageLayout oldLayout, VkImageLayout	 newLayout);

void copyBuffer(VkDevice& device, VkQueue& queue, VkCommandPool& commandPool, VkBuffer& src, VkBuffer& dst, VkDeviceSize size);

void createBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferDeviceMemory, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
template <typename T>
void createAndUploadBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& queue, VkCommandPool& commandPool, std::vector<T> data, VkBuffer& buffer, VkDeviceMemory& bufferDeviceMemory, VkBufferUsageFlags bufferUsageFlags);

uint32_t findMemoryTypeIndex(VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
uint32_t getSwapchainImageCount(const VkDevice & device, const VkSwapchainKHR & swapchain);
std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance & instance);
void createShaderModule(const std::vector<char>&code, VkShaderModule * shaderModule, VkDevice & device);

#ifdef _DEBUG
void printLayerPropertiesInfo();
void printExtensionPropertiesInfo();
void printPhysicalDevicesInfo(VkInstance & instance, VkSurfaceKHR & surface);
#endif

#include "VulkanUtils.tpp"
