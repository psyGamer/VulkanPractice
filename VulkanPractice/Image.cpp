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
	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	ASSERT_VK(vkAllocateCommandBuffers(m_Device, &commandBufferAllocateInfo, &commandBuffer));

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	ASSERT_VK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

	VkImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.oldLayout = m_ImageLayout;
	imageMemoryBarrier.newLayout = layout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = m_Image;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	if (m_ImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	} else if (m_ImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	} else {
		std::cerr << "Layout transition not yet supported!";
		throw std::logic_error("Layout transition not yet supported!");
	}

	vkCmdPipelineBarrier(commandBuffer, 
		VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
		0, nullptr, 0, nullptr, 1, &imageMemoryBarrier
	);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(m_Device, commandPool, 1, &commandBuffer);

	m_ImageLayout = layout;
}

void Image::WriteBufferToImage(VkCommandPool& commandPool, VkQueue& queue, VkBuffer& buffer) {
	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	ASSERT_VK(vkAllocateCommandBuffers(m_Device, &commandBufferAllocateInfo, &commandBuffer));

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	ASSERT_VK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

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

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(m_Device, commandPool, 1, &commandBuffer);
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

	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageCreateInfo.extent = { GetWidth(), GetHeight(), 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = m_ImageLayout;

	ASSERT_VK(vkCreateImage(m_Device, &imageCreateInfo, nullptr, &m_Image));

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(m_Device, m_Image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	ASSERT_VK(vkAllocateMemory(m_Device, &memoryAllocateInfo, nullptr, &m_DeviceImageMemory));

	vkBindImageMemory(m_Device, m_Image, m_DeviceImageMemory, 0);

	ChangeLayout(commandPool, queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	WriteBufferToImage(commandPool, queue, stagingBuffer);
	ChangeLayout(commandPool, queue, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	VkImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.image = m_Image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	ASSERT_VK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_ImageView));

	VkSamplerCreateInfo samplerCreateInfo;
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_FALSE;
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
