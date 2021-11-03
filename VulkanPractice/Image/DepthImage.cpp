#include "DepthImage.h"

#define CHECK_IF_CREATED\
	if (m_Created) {\
		std::cerr << "This image was already created yet!";\
		throw std::logic_error("This image isn't loaded yet!");\
	}

static VkFormat findDepthFormat(VkPhysicalDevice & physicalDevice) {
	std::vector<VkFormat> possibleFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,

		VK_FORMAT_D32_SFLOAT
	};

	return findSupportedFormat(physicalDevice, possibleFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

DepthImage::DepthImage() { }

void DepthImage::Create(VkDevice& device, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& queue, uint32_t width, uint32_t height) {
	if (m_Created) {
		std::cerr << "This image was already created!";
		throw std::logic_error("This image was already created!");
	}

	m_Device = device;

	VkFormat depthFormat = findDepthFormat(physicalDevice);

	createImage(device, physicalDevice, width, height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory);
	createImageView(device, m_Image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, m_ImageView);
	changeImageLayout(device, m_Image, depthFormat, commandPool, queue, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	m_Created = true;
}

void DepthImage::Destory() {
	if (!m_Created)
		return;

	vkDestroyImageView(m_Device, m_ImageView, nullptr);
	vkDestroyImage(m_Device, m_Image, nullptr);
	vkFreeMemory(m_Device, m_ImageMemory, nullptr);

	m_Image = VK_NULL_HANDLE;
	m_ImageView = VK_NULL_HANDLE;
	m_ImageMemory = VK_NULL_HANDLE;

	m_Device = VK_NULL_HANDLE;

	m_Created = false;
}

VkImageView DepthImage::GetImageView() {
	return m_ImageView;
}

VkAttachmentDescription DepthImage::GetDepthAttachmentDescription(VkPhysicalDevice& physicalDevice) {
	VkAttachmentDescription depthAttachment;
	depthAttachment.flags = 0;
	depthAttachment.format = findDepthFormat(physicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	return depthAttachment;
}

VkPipelineDepthStencilStateCreateInfo DepthImage::GetDepthStencilCreateInfoOpaque() {
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
	depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCreateInfo.pNext = nullptr;
	depthStencilStateCreateInfo.flags = 0;
	depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.front = {};
	depthStencilStateCreateInfo.back = {};
	depthStencilStateCreateInfo.minDepthBounds = 0.0f;
	depthStencilStateCreateInfo.maxDepthBounds = 1.0f;

	return depthStencilStateCreateInfo;
}

