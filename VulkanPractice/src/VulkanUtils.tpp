#include <iostream>

template <typename T>
void createAndUploadBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& queue, VkCommandPool& commandPool, std::vector<T> bufferData, VkBuffer& buffer, VkDeviceMemory& bufferDeviceMemory, VkBufferUsageFlags bufferUsageFlags) {
	VkDeviceSize bufferSize = sizeof(T) * bufferData.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(device, physicalDevice, stagingBuffer, stagingBufferMemory, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	void* data;
	ASSERT_VK(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data));
	memcpy(data, bufferData.data(), bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(device, physicalDevice, buffer, bufferDeviceMemory, bufferSize,
		bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	copyBuffer(device, queue, commandPool, stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}