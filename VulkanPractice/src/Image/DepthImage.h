#pragma once

#include "../VulkanUtils.h"

class DepthImage {

private:
	bool m_Created = false;

	VkImage m_Image = VK_NULL_HANDLE;
	VkImageView m_ImageView = VK_NULL_HANDLE;

	VkDevice m_Device = VK_NULL_HANDLE;
	VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;

public:
	DepthImage();

	void Create(VkDevice& device, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& queue, uint32_t width, uint32_t height);
	void Destory();

	VkImageView GetImageView();

	static VkAttachmentDescription GetDepthAttachmentDescription(VkPhysicalDevice& physicalDevice);
	static VkPipelineDepthStencilStateCreateInfo GetDepthStencilCreateInfoOpaque();
};