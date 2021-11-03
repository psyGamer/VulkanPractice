#include "VulkanUtils.h"

#include <iostream>

// Vulkan Helper Functions

bool isFormatSupported(VkPhysicalDevice& physicalDevice, VkFormat format, VkImageTiling imageTiling, VkFormatFeatureFlags featureFlags) {
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

	if (imageTiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & featureFlags) == featureFlags) {
		return true;
	} else if (imageTiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & featureFlags) == featureFlags) {
		return true;
	}

	return false;
}

VkFormat findSupportedFormat(VkPhysicalDevice& physicalDevice, const std::vector<VkFormat>& formats, VkImageTiling imageTiling, VkFormatFeatureFlags featureFlags) {
	for (auto format : formats) {
		if (isFormatSupported(physicalDevice, format, imageTiling, featureFlags))
			return format;
	}

	std::cerr << "No supported format found!";
	throw std::runtime_error("No supported format found!");
}

bool isStencilFormat(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkCommandBuffer startSingleTimeCommandBuffer(VkDevice& device, VkCommandPool& commandPool) {
	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	ASSERT_VK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	ASSERT_VK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

	return commandBuffer;
}

void endSingleTimeCommandBuffer(VkDevice& device, VkCommandBuffer& commandBuffer, VkQueue& queue, VkCommandPool& commandPool) {
	ASSERT_VK(vkEndCommandBuffer(commandBuffer));

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

	ASSERT_VK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE)); //TODO use transfer only queue

	vkQueueWaitIdle(queue);
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void createImage(VkDevice& device, VkPhysicalDevice& physicalDevice, uint32_t width, uint32_t height, VkFormat imageFormat, VkImageTiling imageTiling, VkImageUsageFlags imageUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkImage& image, VkDeviceMemory& imageMemory) {

	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = imageFormat;
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = imageTiling;
	imageCreateInfo.usage = imageUsageFlags;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	ASSERT_VK(vkCreateImage(device, &imageCreateInfo, nullptr, &image));

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	ASSERT_VK(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &imageMemory));


	vkBindImageMemory(device, image, imageMemory, 0);
}

void createImageView(VkDevice& device, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView) {
	VkImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	ASSERT_VK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView));
}

void changeImageLayout(VkDevice& device, VkImage& image, VkFormat format, VkCommandPool& commandPool, VkQueue& queue, VkImageLayout oldLayout, VkImageLayout newLayout) {
	auto commandBuffer = startSingleTimeCommandBuffer(device, commandPool);

	VkImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (isStencilFormat(format)) {
			imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	} else {
		std::cerr << "Layout transition not yet supported!";
		throw std::logic_error("Layout transition not yet supported!");
	}

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
		0, nullptr, 0, nullptr, 1, &imageMemoryBarrier
	);

	endSingleTimeCommandBuffer(device, commandBuffer, queue, commandPool);
}

void copyBuffer(VkDevice& device, VkQueue& queue, VkCommandPool& commandPool, VkBuffer& src, VkBuffer& dst, VkDeviceSize size) {

	auto commandBuffer = startSingleTimeCommandBuffer(device, commandPool);

	VkBufferCopy bufferCopy;
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = size;

	vkCmdCopyBuffer(commandBuffer, src, dst, 1, &bufferCopy);

	endSingleTimeCommandBuffer(device, commandBuffer, queue, commandPool);
}

void createBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferDeviceMemory, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags) {
	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.usage = bufferUsageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	ASSERT_VK(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer));

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	ASSERT_VK(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &bufferDeviceMemory));
	ASSERT_VK(vkBindBufferMemory(device, buffer, bufferDeviceMemory, 0));
}

uint32_t findMemoryTypeIndex(VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties); //TODO civ

	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	std::cerr << "Found no correct memory type!";
	throw std::runtime_error("Found no correct memory type!");
}

uint32_t getSwapchainImageCount(const VkDevice& device, const VkSwapchainKHR& swapchain) {
	uint32_t swapchainImageCount;
	ASSERT_VK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr));

	return swapchainImageCount;
}

std::vector<VkPhysicalDevice> physicalDevices;

std::vector< VkPhysicalDevice> getPhysicalDevices(VkInstance& instance) {
	if (physicalDevices.size() > 0) {
		return physicalDevices;
	}

	uint32_t physicalDeviceCount;
	ASSERT_VK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
	physicalDevices.resize(physicalDeviceCount);
	ASSERT_VK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

	return physicalDevices;
}

// Print Device Stats



#ifdef _DEBUG

// Converts a Vulkan version int to a human readable stirng
#define VERION_STR_VK(ver)\
	(std::to_string(VK_API_VERSION_MAJOR(ver))\
		+ std::string(".") + std::to_string(VK_API_VERSION_MINOR(ver))\
		+ std::string(".") + std::to_string(VK_API_VERSION_PATCH(ver)))
// Returns "Yes" or "No" depending on, if the specified bit is set
#define YES_NO_BIT(val, bit) (((val & bit) != 0) ? "Yes" : "No")

void printLayerPropertiesInfo() {
	uint32_t layerCount;
	ASSERT_VK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
	std::vector< VkLayerProperties> layerProperties;
	layerProperties.resize(layerCount);
	ASSERT_VK(vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data()));

	std::cout << "    --- Layer Information ---" << std::endl << std::endl;
	std::cout << "Layer Count: " << layerCount << std::endl << std::endl;

	for (int i = 0; i < layerCount; i++) {
		std::cout << "Layer #" << i << ":" << std::endl;

		std::cout << "\tName:                   " << layerProperties[i].layerName << std::endl;
		std::cout << "\tDescription:            " << layerProperties[i].description << std::endl;
		std::cout << "\tSpecification Version:  " << layerProperties[i].specVersion << std::endl;
		std::cout << "\tImplementation Version: " << layerProperties[i].implementationVersion << std::endl;

		std::cout << std::endl;
	}
}

void printExtensionPropertiesInfo() {
	uint32_t extensionCount;
	ASSERT_VK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
	std::vector< VkExtensionProperties> extensionProperties;
	extensionProperties.resize(extensionCount);
	ASSERT_VK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data()));

	std::cout << "    --- Extension Information ---" << std::endl << std::endl;
	std::cout << "Extension Count: " << extensionCount << std::endl << std::endl;

	for (int i = 0; i < extensionCount; i++) {
		std::cout << "Extension #" << i << ":" << std::endl;

		std::cout << "\tName:                   " << extensionProperties[i].extensionName << std::endl;
		std::cout << "\tSpecification Version:  " << extensionProperties[i].specVersion << std::endl;

		std::cout << std::endl;
	}
}

void printPhysicalDevicesInfo(VkInstance& instance, VkSurfaceKHR& surface) {
	auto physicalDevices = getPhysicalDevices(instance);

	std::cout << "    --- Device Information ---" << std::endl << std::endl;
	std::cout << "Device Count: " << physicalDevices.size() << std::endl << std::endl;

	for (int i = 0; i < physicalDevices.size(); i++) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

		std::cout << "Device #" << i << ":" << std::endl;

		std::cout << "\tProperties:" << std::endl;
		std::cout << "\t\tName:    " << properties.deviceName << std::endl;
		std::cout << "\t\tType:    " << properties.deviceType << std::endl;
		std::cout << "\t\tAPI Version:    " << VERION_STR_VK(properties.apiVersion) << std::endl;
		std::cout << "\t\tDriver Version: " << properties.driverVersion << std::endl;
		std::cout << "\t\tVendor ID:      " << properties.vendorID << std::endl;
		std::cout << "\t\tDevice ID:      " << properties.deviceID << std::endl;

		std::cout << std::endl;

		std::cout << "\t\tDiscrete Queue Priorites: " << properties.limits.discreteQueuePriorities << std::endl;

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

		std::cout << "\tFeatures:" << std::endl;
		std::cout << "\t\tGeometry Shader:     " << (features.geometryShader ? "Yes" : "No") << std::endl;
		std::cout << "\t\tTessellation Shader: " << (features.tessellationShader ? "Yes" : "No") << std::endl;

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, nullptr);
		std::vector< VkQueueFamilyProperties>queueFamilyProperties;
		queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, queueFamilyProperties.data());

		std::cout << "\tQueue Properties:" << std::endl;
		std::cout << "\t\tFamily Count: " << queueFamilyCount << std::endl;

		for (int i = 0; i < queueFamilyCount; i++) {
			std::cout << std::endl;
			std::cout << "\t\tQueue Family #" << i << ":" << std::endl;

			std::cout << "\t\t\tGraphics:        " << YES_NO_BIT(queueFamilyProperties[i].queueFlags, VK_QUEUE_GRAPHICS_BIT) << std::endl;
			std::cout << "\t\t\tCompute:         " << YES_NO_BIT(queueFamilyProperties[i].queueFlags, VK_QUEUE_COMPUTE_BIT) << std::endl;
			std::cout << "\t\t\tTransfer:        " << YES_NO_BIT(queueFamilyProperties[i].queueFlags, VK_QUEUE_TRANSFER_BIT) << std::endl;
			std::cout << "\t\t\tSparse Binding:  " << YES_NO_BIT(queueFamilyProperties[i].queueFlags, VK_QUEUE_SPARSE_BINDING_BIT) << std::endl;
			std::cout << "\t\t\tProtected:       " << YES_NO_BIT(queueFamilyProperties[i].queueFlags, VK_QUEUE_PROTECTED_BIT) << std::endl;

			std::cout << std::endl;

			std::cout << "\t\t\tQueue Count:                    " << queueFamilyProperties[i].queueCount << std::endl;
			std::cout << "\t\t\tTimestamp Bits:                 " << queueFamilyProperties[i].timestampValidBits << std::endl;
			std::cout << "\t\t\tMin Image Transfer Granularity: "
				<< queueFamilyProperties[i].minImageTransferGranularity.width << ", "
				<< queueFamilyProperties[i].minImageTransferGranularity.height << ", "
				<< queueFamilyProperties[i].minImageTransferGranularity.depth << std::endl;
		}

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[i], surface, &surfaceCapabilities);

		std::cout << "\tSurface Capabilities:" << std::endl;

		std::cout << "\t\tMin Image Count: " << surfaceCapabilities.minImageCount << std::endl;
		std::cout << "\t\tMax Image Count: " << surfaceCapabilities.maxImageCount << std::endl;
		std::cout << "\t\tCurrent Extent: " << surfaceCapabilities.currentExtent.width << ", " << surfaceCapabilities.currentExtent.height << std::endl;
		std::cout << "\t\tMin Image Extent: " << surfaceCapabilities.minImageExtent.width << ", " << surfaceCapabilities.minImageExtent.height << std::endl;
		std::cout << "\t\tMax Image Extent: " << surfaceCapabilities.maxImageExtent.width << ", " << surfaceCapabilities.maxImageExtent.height << std::endl;
		std::cout << "\t\tMax Image Array Layers: " << surfaceCapabilities.maxImageArrayLayers << std::endl;
		std::cout << "\t\tSupported Transforms: " << surfaceCapabilities.supportedTransforms << std::endl;
		std::cout << "\t\tCurrent Transform: " << surfaceCapabilities.currentTransform << std::endl;
		std::cout << "\t\tSupported Composite Alpha: " << surfaceCapabilities.supportedCompositeAlpha << std::endl;
		std::cout << "\t\tSupported Usage Flags: " << surfaceCapabilities.supportedUsageFlags << std::endl;

		uint32_t surfaceFormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[i], surface, &surfaceFormatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		surfaceFormats.resize(surfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[i], surface, &surfaceFormatCount, surfaceFormats.data());

		std::cout << "\tSurface Formats:" << std::endl << "\t\t";

		for (auto surfaceFormat : surfaceFormats) {
			std::cout << surfaceFormat.format << " ";
		}

		std::cout << std::endl;

		uint32_t surfacePresentationModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevices[i], surface, &surfacePresentationModeCount, nullptr);
		std::vector<VkPresentModeKHR> surfacePresentationModes;
		surfacePresentationModes.resize(surfacePresentationModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevices[i], surface, &surfacePresentationModeCount, surfacePresentationModes.data());

		std::cout << "\tSurface Presentation Modes:" << std::endl << "\t\t";

		for (auto surfacePresentationMode : surfacePresentationModes) {
			std::cout << surfacePresentationMode << " ";
		}

		std::cout << std::endl;
	}
}

#undef VERION_STR_VK
#undef YES_NO_BIT

#endif
