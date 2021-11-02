#pragma once

#include <string>
#include <stb_image.h>

#include "VulkanUtils.h"

class Image {

private:
	bool m_Loaded, m_Uploaded = false;
	int m_Width, m_Height, m_Channels;
	stbi_uc* m_pPixels;

	VkImage m_Image;
	VkImageView m_ImageView;
	VkImageLayout m_ImageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	VkDevice m_Device;
	VkDeviceMemory m_DeviceImageMemory;

	VkSampler m_Sampler;

private:

	void ChangeLayout(VkCommandPool& commandPool, VkQueue& queue, VkImageLayout layout);
	void WriteBufferToImage(VkCommandPool& commandPool, VkQueue& queue, VkBuffer& buffer);
		
public:
	Image();
	Image(const std::string& filePath);

	~Image();

	Image(const Image &) = delete;
	Image(Image &&) = delete;

	Image operator=(const Image &) = delete;
	Image operator=(Image &&) = delete;

	uint32_t GetHeight();
	uint32_t GetWidth();
	int GetChannels();
	int GetSizeInBytes();

	stbi_uc* GetRawPixels();

	VkSampler& GetSampler();
	VkImageView& GetImageView();

	void Upload(VkDevice& device, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& queue);

	void LoadImage(const std::string& filePath);
	void Destory();
};