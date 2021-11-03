#pragma once

#include "Vulkan.h"

struct SwapchainSupportInfo {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	std::vector<VkSurfaceFormatKHR> supportedSurfaceFormats;
	std::vector<VkPresentModeKHR> supportedPresentModes;
};

class Swapchain {

private:
	bool m_Created = false;

	GLFWwindow* m_Window = nullptr;

	VkDevice m_Device = VK_NULL_HANDLE;
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

	VkPhysicalDevice m_PhysicalDevice;
	VkSurfaceKHR m_Surface;

private:
	static SwapchainSupportInfo QuerySwapchainSupportInfo(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

	static VkExtent2D ChoseExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	static VkSurfaceFormatKHR ChoseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& supportedSurfaceFormats);
	static VkPresentModeKHR ChosePresentMode(const std::vector<VkPresentModeKHR>& supportedPresentModes);

	void CreateSwapchain(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, uint32_t width, uint32_t height, const VkSwapchainKHR& oldSwapchain);

public:
	Swapchain(GLFWwindow* window, VkDevice device);

	Swapchain(const Swapchain&) = delete;
	Swapchain& operator=(const Swapchain&) = delete;
	Swapchain(Swapchain &&) = default;
	Swapchain& operator=(Swapchain &&) = default;

	void Create(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, uint32_t width, uint32_t height);
	void Recreate(uint32_t width, uint32_t height);
	void Destory();

	inline VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
};

