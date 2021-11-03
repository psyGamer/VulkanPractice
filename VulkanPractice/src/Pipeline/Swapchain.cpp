#include "Swapchain.h"

#include <cstdint>
#include <algorithm>

Swapchain::Swapchain(GLFWwindow* window, VkDevice device)
	: m_Window(window), m_Device(device) { }

SwapchainSupportInfo Swapchain::QuerySwapchainSupportInfo(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface) {
	SwapchainSupportInfo supportInfo;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &supportInfo.surfaceCapabilities);

	uint32_t surfaceFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
	supportInfo.supportedSurfaceFormats.resize(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, supportInfo.supportedSurfaceFormats.data());

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	supportInfo.supportedPresentModes.resize(surfaceFormatCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, supportInfo.supportedPresentModes.data());

	return supportInfo;
}

VkExtent2D Swapchain::ChoseExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
	if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
		return surfaceCapabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

		return actualExtent;
	}
}
VkSurfaceFormatKHR Swapchain::ChoseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& supportedSurfaceFormats) {
	for (const auto& surfaceFormat : supportedSurfaceFormats) {
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			return surfaceFormat; // Optimal surface format
	}

	return supportedSurfaceFormats[0]; // No optimal surface format was found;
}
VkPresentModeKHR Swapchain::ChosePresentMode(const std::vector<VkPresentModeKHR>& supportedPresentModes) {
	for (const auto& presentMode : supportedPresentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode; // Optimal present mode
	}

	return VK_PRESENT_MODE_FIFO_KHR; // No optimal present mode was found
}

void Swapchain::CreateSwapchain(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, uint32_t width, uint32_t height, const VkSwapchainKHR& oldSwapchain) {
	ASSERT(m_Created, "Swapchain already created!");

	SwapchainSupportInfo supportInfo = QuerySwapchainSupportInfo(physicalDevice, surface);

	VkExtent2D extent = ChoseExtent(m_Window, supportInfo.surfaceCapabilities);
	VkSurfaceFormatKHR surfaceFormat = ChoseSurfaceFormat(supportInfo.supportedSurfaceFormats);
	VkPresentModeKHR presentMode = ChosePresentMode(supportInfo.supportedPresentModes);

	uint32_t imageCount = supportInfo.surfaceCapabilities.minImageCount + 1;

	if (supportInfo.surfaceCapabilities.maxImageCount > 0 && imageCount > supportInfo.surfaceCapabilities.maxImageCount)
		imageCount = supportInfo.surfaceCapabilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = surface;

	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //TODO civ
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;

	swapchainCreateInfo.preTransform = supportInfo.surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;

	swapchainCreateInfo.oldSwapchain = oldSwapchain;

	ASSERT_VK(vkCreateSwapchainKHR(m_Device, &swapchainCreateInfo, nullptr, &m_Swapchain), "Failed to create swapchain!");

	m_Created = true;
}

void Swapchain::Create(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, uint32_t width, uint32_t height) {
	CreateSwapchain(physicalDevice, surface, width, height, VK_NULL_HANDLE);

	m_PhysicalDevice = physicalDevice;
	m_Surface = surface;
}
void Swapchain::Recreate(uint32_t width, uint32_t height) {
	ASSERT(!m_Created, "Swapchain must be created first, before it can be recreated!");

	Destory();
	CreateSwapchain(m_PhysicalDevice, m_Surface, width, height, m_Swapchain);
}

void Swapchain::Destory() {
	if (!m_Created)
		return;

	vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

	m_Swapchain = VK_NULL_HANDLE;
	m_Created = false;
}