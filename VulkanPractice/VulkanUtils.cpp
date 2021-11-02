#include "VulkanUtils.h"

#include <iostream>
#include <fstream>

// Gleneral Helper Functions

std::vector<char> readFile(const std::string& filePath) {
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);

	if (file) {
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> fileBuffer(fileSize);

		file.seekg(0);
		file.read(fileBuffer.data(), fileSize);
		file.close();

		return fileBuffer;
	}

	std::cerr << "Failed to open file: " << filePath;
	throw std::runtime_error("Failed to open file!");
}

// Vulkan Helper Functions

void copyBuffer(VkInstance& instance, VkDevice& device, VkQueue& queue, VkCommandPool& commandPool, VkBuffer& src, VkBuffer& dst, VkDeviceSize size) {

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

	VkBufferCopy bufferCopy;
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = size;

	vkCmdCopyBuffer(commandBuffer, src, dst, 1, &bufferCopy);

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

void createBuffer(VkInstance& instance, VkDevice& device, VkBuffer& buffer, VkDeviceMemory& bufferDeviceMemory, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags) {
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
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(instance, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	ASSERT_VK(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &bufferDeviceMemory));
	ASSERT_VK(vkBindBufferMemory(device, buffer, bufferDeviceMemory, 0));
}

uint32_t findMemoryTypeIndex(VkInstance& instance, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(getPhysicalDevices(instance)[0], &physicalDeviceMemoryProperties); //TODO civ

	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	std::cerr << "Found no correct memory type!";
	throw std::runtime_error("Found no correct memory type!");
}

uint32_t getSwapchainImageCount(VkDevice& device, VkSwapchainKHR& swapchain) {
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

void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule, VkDevice& device) {
	VkShaderModuleCreateInfo shaderCreateInfo;
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.pNext = nullptr;
	shaderCreateInfo.flags = 0;
	shaderCreateInfo.codeSize = code.size();
	shaderCreateInfo.pCode = (uint32_t*)code.data();

	ASSERT_VK(vkCreateShaderModule(device, &shaderCreateInfo, nullptr, shaderModule));
}

// Print Device Stats



#ifdef _DEBUG
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
#endif