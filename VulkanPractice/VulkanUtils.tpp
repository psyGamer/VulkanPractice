#include <iostream>

template <typename T>
void createAndUploadBuffer(VkInstance& instance, VkDevice& device, VkQueue& queue, VkCommandPool& commandPool, std::vector<T> data, VkBuffer& buffer, VkDeviceMemory& bufferDeviceMemory, VkBufferUsageFlags bufferUsageFlags) {
	VkDeviceSize bufferSize = sizeof(T) * data.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(instance, device, stagingBuffer, stagingBufferMemory, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	void* rawData;
	ASSERT_VK(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &rawData));
	memcpy(rawData, data.data(), bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(instance, device, buffer, bufferDeviceMemory, bufferSize,
		bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	copyBuffer(instance, device, queue, commandPool, stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}