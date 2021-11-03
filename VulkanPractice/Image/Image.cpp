#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CHECK_IF_LOADED\
	if (!m_Loaded) {\
		std::cerr << "This image isn't loaded yet!";\
		throw std::logic_error("This image isn't loaded yet!");\
	}

Image::Image() {
	m_Loaded = false;

	m_Width = 0;
	m_Height = 0;
	m_Channels = 0;

	m_pPixels = nullptr;
}

Image::Image(const std::string& filePath) {
	LoadImage(filePath);
}

Image::~Image() {
	Destory();
}

uint32_t Image::GetHeight() {
	CHECK_IF_LOADED

	return (uint32_t)m_Height;
}
uint32_t Image::GetWidth() {
	CHECK_IF_LOADED

	return (uint32_t)m_Width;
}
int Image::GetChannels() {
	CHECK_IF_LOADED

	return 4;
}
int Image::GetSizeInBytes() {
	CHECK_IF_LOADED

	return GetWidth() * GetHeight() * GetChannels();
}

stbi_uc* Image::GetRawPixels() {
	CHECK_IF_LOADED

	return m_pPixels;
}

VkSampler& Image::GetSampler() {
	CHECK_IF_LOADED

	return m_Sampler;
}

VkImageView& Image::GetImageView() {
	CHECK_IF_LOADED

	return m_ImageView;
}

void Image::ChangeLayout(VkCommandPool& commandPool, VkQueue& queue, VkImageLayout layout) {
	changeImageLayout(m_Device, m_Image, VK_FORMAT_R8G8B8A8_UNORM, commandPool, queue, m_ImageLayout, layout);

	m_ImageLayout = layout;
}

void Image::WriteBufferToImage(VkCommandPool& commandPool, VkQueue& queue, VkBuffer& buffer) {
	auto commandBuffer = startSingleTimeCommandBuffer(m_Device, commandPool);

	VkBufferImageCopy bufferImageCopy;
	bufferImageCopy.bufferOffset = 0;
	bufferImageCopy.bufferRowLength = 0;
	bufferImageCopy.bufferImageHeight = 0;
	bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopy.imageSubresource.mipLevel = 0;
	bufferImageCopy.imageSubresource.baseArrayLayer = 0;
	bufferImageCopy.imageSubresource.layerCount = 1;
	bufferImageCopy.imageOffset = { 0, 0, 0 };
	bufferImageCopy.imageExtent = { GetWidth(), GetHeight(), 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

	endSingleTimeCommandBuffer(m_Device, commandBuffer, queue, commandPool);
}

void Image::Upload(VkDevice& device, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& queue) {
	CHECK_IF_LOADED

	if (m_Uploaded) {
		std::cerr << "This image was already uploaded!";
		throw std::logic_error("This image was already uploaded!");
	}

	m_Device = device;

	VkDeviceSize imageSize = GetSizeInBytes();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(m_Device, physicalDevice, stagingBuffer, stagingBufferMemory, imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	void* data;
	vkMapMemory(m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, GetRawPixels(), imageSize);
	vkUnmapMemory(m_Device, stagingBufferMemory);

	createImage(m_Device, physicalDevice, GetWidth(), GetHeight(),
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_DeviceImageMemory
	);

	ChangeLayout(commandPool, queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	WriteBufferToImage(commandPool, queue, stagingBuffer);
	ChangeLayout(commandPool, queue, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	createImageView(m_Device, m_Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView);

	VkSamplerCreateInfo samplerCreateInfo;
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = 16;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	ASSERT_VK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_Sampler));

	m_Uploaded = true;
}

void Image::LoadImage(const std::string& filePath) {
	m_pPixels = stbi_load(filePath.c_str(), &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha);

	if (m_pPixels == nullptr) {
		std::cerr << "Could not load image: " + filePath;
		throw std::invalid_argument("Could not load image: " + filePath);
	}

	m_Loaded = true;
}

void Image::Destory() {
	if (!m_Loaded)
		return;

	stbi_image_free(m_pPixels);

	m_Loaded = false;

	if (!m_Uploaded)
		return;

	vkDestroySampler(m_Device, m_Sampler, nullptr);
	vkDestroyImageView(m_Device, m_ImageView, nullptr);

	vkDestroyImage(m_Device, m_Image, nullptr);
	vkFreeMemory(m_Device, m_DeviceImageMemory, nullptr);

	m_Uploaded = false;
}

#undef CHECK_IF_LOADED
