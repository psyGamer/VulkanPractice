#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include <fstream>

#include <stdlib.h>
#include <time.h>

#pragma warning(disable : 26812)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Ensures that VkResult is VK_SUCCESS
#ifndef ASSERT_VK 
VkResult __assert_vk_result;
#endif
#define ASSERT_VK(exp)\
	__assert_vk_result = (exp);\
	if (__assert_vk_result != VK_SUCCESS) {\
		std::cerr << "An unexpected error occurred on " << __FILE__ << ":" << __LINE__ << "\n\tError Code: " << __assert_vk_result;\
		__debugbreak();\
	}

// Use at your own risk
#define GET_VK(vkName, varName, ...)\
	Vk ## vkName varName;\
	vkGet ## vkName ## (__VA_ARGS__, &varName)
// Converts a Vulkan version int to a human readable stirng
#define VERION_STR_VK(ver)\
	(std::to_string(VK_API_VERSION_MAJOR(ver))\
		+ std::string(".") + std::to_string(VK_API_VERSION_MINOR(ver))\
		+ std::string(".") + std::to_string(VK_API_VERSION_PATCH(ver)))
// Returns "Yes" or "No" depending on, if the specified bit is set
#define YES_NO_BIT(val, bit) (((val & bit) != 0) ? "Yes" : "No")

#ifdef _DEBUG
void printLayerPropertiesInfo();
void printExtensionPropertiesInfo();
void printPhysicalDevicesInfo();
#endif

void recreateSwapchain();

std::vector<char> readFile(const std::string& filePath);

uint32_t getSwapchainImageCount();
std::vector<VkPhysicalDevice> getPhysicalDevices();
void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

VkInstance instance;
VkSurfaceKHR surface;
VkDevice device;
VkQueue queue;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;

std::vector<VkPhysicalDevice> physicalDevices;

std::vector<VkImageView> imageViews;
std::vector<VkFramebuffer> frameBuffers;
std::vector<VkCommandBuffer> commandBuffers;

VkShaderModule shaderModuleVert;
VkShaderModule shaderModuleFrag;

VkPipelineLayout pipelineLayout;
VkRenderPass renderPass;

VkPipeline pipeline;

VkCommandPool commandPool;

VkBuffer vertexBuffer;
VkBuffer indexBuffer;
VkBuffer uniformBuffer;
VkDeviceMemory vertexBufferDeviceMemory;
VkDeviceMemory indexBufferDeviceMemory;
VkDeviceMemory uniformBufferDeviceMemory;

VkDescriptorSetLayout descriptorSetLayout;
VkDescriptorPool descriptorPool;
VkDescriptorSet descriptorSet;

VkSemaphore semaphoreImageAvailable;
VkSemaphore semaphoreRenderingDone;

GLFWwindow* window;

uint32_t windowWidth = 400, windowHeight = 300;

const VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM;

class Camera {
public:
	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 up;

	float yaw, pitch;
	float fov;

	Camera() {
		pos = glm::vec3(0.0f, 1.0f, 1.0f);
		front = glm::vec3(0.0f, 0.0f, -1.0f);
		up = glm::vec3(0.0f, -1.0f, 0.0f);

		fov = 70.0f;
	}
};

class Vertex {
public:
	glm::vec2 pos;
	glm::vec3 color;

	Vertex(glm::vec2 pos, glm::vec3 color)
		: pos(pos), color(color) { }

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};

const float squreSize = .5f;

Camera cam = Camera();

glm::mat4 modelViewProj;

std::vector<Vertex> vertices = {
	Vertex(glm::vec2{ -1.0f, -1.0f } * squreSize, { 1.0f, 0.0f, 1.0f }), // 0 Top Left
	Vertex(glm::vec2{  1.0f, -1.0f } * squreSize, { 0.0f, 0.0f, 1.0f }), // 1 Top Right
	Vertex(glm::vec2{ -1.0f,  1.0f } * squreSize, { 0.0f, 0.0f, 1.0f }), // 2 Bottom Left
	Vertex(glm::vec2{  1.0f,  1.0f } * squreSize, { 0.0f, 1.0f, 0.0f }), // 3 Bottom Right
};

std::vector<uint32_t> indices = {
	0, 3, 2,
	0, 1, 3
};

void onWindowResized(GLFWwindow * window, int width, int height) {
	if (width == 0 || height == 0)
		return;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[0], surface, &surfaceCapabilities);

	if (width > surfaceCapabilities.maxImageExtent.width)
		width = surfaceCapabilities.maxImageExtent.width;
	if (height > surfaceCapabilities.maxImageExtent.height)
		height = surfaceCapabilities.maxImageExtent.height;

	windowWidth = width;
	windowHeight = height;

	recreateSwapchain();
}

bool firstMouse = true;
float lastMouseX = windowWidth / 2.0f, lastMouseY = windowHeight / 2.0f;

void onMouseMove(GLFWwindow* window, double xpos, double ypos) {
	const float sensitivity = 0.1f;

	if (firstMouse) {
		lastMouseX = xpos;
		lastMouseY = ypos;
		firstMouse = false;
	}

	float xOffset = (xpos - lastMouseX) * sensitivity;
	float yOffset = (ypos - lastMouseY) * sensitivity;

	lastMouseX = xpos;
	lastMouseY = ypos;

	cam.yaw -= xOffset;
	cam.pitch += yOffset;

	if (cam.pitch > 89.0f)
		cam.pitch = 89.0f;
	if (cam.pitch < -89.0f)
		cam.pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
	direction.y = sin(glm::radians(cam.pitch));
	direction.z = sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));

	cam.front = glm::normalize(direction);
}

void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
	const float sensitivity = 2.0f;
	cam.fov -= (float)yoffset * sensitivity;
	if (cam.fov < 1.0f)
		cam.fov = 1.0f;
	if (cam.fov > 150.0f)
		cam.fov = 150.0f;
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	const float cameraSpeed = 0.05f; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cam.pos += cameraSpeed * cam.front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cam.pos -= cameraSpeed * cam.front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cam.pos -= glm::normalize(glm::cross(cam.front, cam.up)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cam.pos += glm::normalize(glm::cross(cam.front, cam.up)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		cam.pos -= glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		cam.pos += glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed;
}

void startGlfw() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(windowWidth, windowHeight, "Vulkan Practice", nullptr, nullptr);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetWindowSizeCallback(window, onWindowResized);
	glfwSetCursorPosCallback(window, onMouseMove);
	glfwSetScrollCallback(window, onMouseScroll);
}

void createInstance() {
	VkApplicationInfo applicationInfo;
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = "Vulkan Tutorial";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	applicationInfo.pEngineName = "ZerfickerEngine_69";
	applicationInfo.engineVersion = VK_MAKE_VERSION(69, 420, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	const std::vector<const char*> enabledInstanceLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	uint32_t glfwExtensionCount;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = enabledInstanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = enabledInstanceLayers.data();
	instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
	instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;

	ASSERT_VK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
}

void createGlfwWindowSurface() {
	ASSERT_VK(glfwCreateWindowSurface(instance, window, nullptr, &surface));
}

void createLogicalDevice() {
	auto physicalDevices = getPhysicalDevices();

	float queuePriorities[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	VkDeviceQueueCreateInfo deviceQueueCreateInfo;
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.pNext = nullptr;
	deviceQueueCreateInfo.flags = 0;
	deviceQueueCreateInfo.queueFamilyIndex = 0; //TODO select "best" queue family
	deviceQueueCreateInfo.queueCount = 1; //TODO civ (check if valid)
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

	VkPhysicalDeviceFeatures usedFeatures{ };

	const std::vector<const char*> enabledDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = enabledDeviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &usedFeatures;

	ASSERT_VK(vkCreateDevice(physicalDevices[0], &deviceCreateInfo, nullptr, &device)); //TODO selected "best" device
}

void initializeQueue() {
	vkGetDeviceQueue(device, 0, 0, &queue); //TODO civ
}

void validateSurfaceSupport() {
	auto physicalDevices = getPhysicalDevices();

	VkBool32 surfaceSupport;
	ASSERT_VK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[0], 0, surface, &surfaceSupport));

	if (!surfaceSupport) {
		std::cerr << "Surface not supported!" << std::endl;
		__debugbreak();

		return;
	}
}

void createSwapchain() {
#ifndef _DEBUG
	VkSurfaceCapabilitiesKHR surfaceCapabilities; // Avoid validation warning when in Release
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[0], surface, &surfaceCapabilities);
#endif
	VkSwapchainCreateInfoKHR swapchainCreateInfo;
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 2; //TODO civ
	swapchainCreateInfo.imageFormat = imageFormat; //TODO civ
	swapchainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR; //TODO civ
	swapchainCreateInfo.imageExtent = VkExtent2D{ windowWidth, windowHeight };
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //TODO civ
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; //TODO civ
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = swapchain;

	ASSERT_VK(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain));
}

void createImageViews() {
	uint32_t swapchainImageCount = getSwapchainImageCount();

	std::vector<VkImage> swapchainImages;
	swapchainImages.resize(swapchainImageCount);
	ASSERT_VK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data()));

	imageViews.resize(swapchainImageCount);

	for (int i = 0; i < swapchainImageCount; i++) {
		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = imageFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		ASSERT_VK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageViews[i]));
	}
}

void createRenderPass() {
	VkAttachmentDescription attachmentDescription;
	attachmentDescription.flags = 0;
	attachmentDescription.format = imageFormat;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference attachmentReference;
	attachmentReference.attachment = 0;
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &attachmentReference;
	subpassDescription.pResolveAttachments = nullptr;
	subpassDescription.pDepthStencilAttachment = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	VkSubpassDependency subpassDependency;
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = 0;

	VkRenderPassCreateInfo renderPassCreateInfo;
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;

	ASSERT_VK(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass));
}

void createDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
	descriptorSetLayoutBinding.binding = 0;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = nullptr;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

	ASSERT_VK(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));
}

void createPipeline() {
	auto shaderCodeVert = readFile("Shaders/vert.spv");
	auto shaderCodeFrag = readFile("Shaders/frag.spv");

	createShaderModule(shaderCodeVert, &shaderModuleVert);
	createShaderModule(shaderCodeFrag, &shaderModuleFrag);

	VkPipelineShaderStageCreateInfo shaderStageCreateInfoVert;
	shaderStageCreateInfoVert.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfoVert.pNext = nullptr;
	shaderStageCreateInfoVert.flags = 0;
	shaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfoVert.module = shaderModuleVert;
	shaderStageCreateInfoVert.pName = "main";
	shaderStageCreateInfoVert.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStageCreateInfoFrag;
	shaderStageCreateInfoFrag.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfoFrag.pNext = nullptr;
	shaderStageCreateInfoFrag.flags = 0;
	shaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfoFrag.module = shaderModuleFrag;
	shaderStageCreateInfoFrag.pName = "main";
	shaderStageCreateInfoFrag.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStates[] = { shaderStageCreateInfoVert, shaderStageCreateInfoFrag };

	auto vertexBindingDescription = Vertex::getBindingDescription();
	auto vertexAttributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.pNext = nullptr;
	vertexInputCreateInfo.flags = 0;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
	vertexInputCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.pNext = nullptr;
	inputAssemblyCreateInfo.flags = 0;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = windowWidth;
	viewport.height = windowHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = { windowWidth, windowHeight };

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.pNext = nullptr;
	viewportStateCreateInfo.flags = 0;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo;
	rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationCreateInfo.pNext = nullptr;
	rasterizationCreateInfo.flags = 0;
	rasterizationCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationCreateInfo.cullMode = VK_CULL_MODE_NONE;
	rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationCreateInfo.depthBiasConstantFactor = VK_FALSE;
	rasterizationCreateInfo.depthBiasClamp = 0.0f;
	rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;
	rasterizationCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo;
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.pNext = nullptr;
	multisampleCreateInfo.flags = 0;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.minSampleShading = 1.0f;
	multisampleCreateInfo.pSampleMask = nullptr;
	multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
	colorBlendAttachmentState.blendEnable = VK_TRUE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.pNext = nullptr;
	colorBlendCreateInfo.flags = 0;
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
	colorBlendCreateInfo.attachmentCount = 1;
	colorBlendCreateInfo.pAttachments = &colorBlendAttachmentState;
	colorBlendCreateInfo.blendConstants[0] = 0.0f;
	colorBlendCreateInfo.blendConstants[1] = 0.0f;
	colorBlendCreateInfo.blendConstants[2] = 0.0f;
	colorBlendCreateInfo.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo layoutCreateInfo;
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pNext = nullptr;
	layoutCreateInfo.flags = 0;
	layoutCreateInfo.setLayoutCount = 1;
	layoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	layoutCreateInfo.pushConstantRangeCount = 0;
	layoutCreateInfo.pPushConstantRanges = nullptr;

	ASSERT_VK(vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &pipelineLayout));

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.pNext = nullptr;
	dynamicStateCreateInfo.flags = 0;
	dynamicStateCreateInfo.dynamicStateCount = 2;
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStates;
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	ASSERT_VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline));
}

void createFrameBuffers() {
	uint32_t swapchainImageCount = getSwapchainImageCount();

	frameBuffers.resize(swapchainImageCount);

	for (int i = 0; i < swapchainImageCount; i++) {
		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &(imageViews[i]);
		framebufferCreateInfo.width = windowWidth;
		framebufferCreateInfo.height = windowHeight;
		framebufferCreateInfo.layers = 1;

		ASSERT_VK(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &(frameBuffers[i])));
	}
}

void createCommandPool() {
	VkCommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = 0; //TODO civ

	ASSERT_VK(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool));
}

void createCommandBuffers() {
	uint32_t swapchainImageCount = getSwapchainImageCount();

	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = swapchainImageCount;

	commandBuffers.resize(swapchainImageCount);

	ASSERT_VK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data()));
}

uint32_t findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevices[0], &physicalDeviceMemoryProperties); //TODO civ

	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	std::cerr << "Found no correct memory type!";
	throw std::runtime_error("Found no correct memory type!");
}

void createBuffer(VkBuffer& buffer, VkDeviceMemory& bufferDeviceMemory, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags) {
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
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	ASSERT_VK(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &bufferDeviceMemory));
	ASSERT_VK(vkBindBufferMemory(device, buffer, bufferDeviceMemory, 0));
}

void copyBuffer(VkBuffer& src, VkBuffer& dst, VkDeviceSize size) {

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

template <typename T>
void createAndUploadBuffer(std::vector<T> data, VkBuffer& buffer, VkDeviceMemory& bufferDeviceMemory, VkBufferUsageFlags bufferUsageFlags) {
	VkDeviceSize bufferSize = sizeof(T) * data.size();
	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(stagingBuffer, stagingBufferMemory, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	void* rawData;
	ASSERT_VK(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &rawData));
	memcpy(rawData, data.data(), bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(buffer, bufferDeviceMemory, bufferSize,
		bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void createVertexBuffer() {
	createAndUploadBuffer(vertices, vertexBuffer, vertexBufferDeviceMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

void createIndexBuffer() {
	createAndUploadBuffer(indices, indexBuffer, indexBufferDeviceMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void createUniformBuffer() {
	VkDeviceSize bufferSize = sizeof(modelViewProj);

	createBuffer(uniformBuffer, uniformBufferDeviceMemory, bufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

void createDescriptorPool() {
	VkDescriptorPoolSize descriptorPoolSize;
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = 1;
	descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
	
	ASSERT_VK(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));
}

void createDescriptorSet() {
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = nullptr;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

	ASSERT_VK(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

	VkDescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = uniformBuffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(modelViewProj);

	VkWriteDescriptorSet writeDescriptorSet;
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = nullptr;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.pImageInfo = nullptr;
	writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
	writeDescriptorSet.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void recordCommandBuffers() {
	uint32_t swapchainImageCount = getSwapchainImageCount();

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	for (int i = 0; i < swapchainImageCount; i++) {
		ASSERT_VK(vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo));

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = frameBuffers[i];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = { windowWidth, windowHeight };
		VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = windowWidth;
		viewport.height = windowHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor;
		scissor.offset = { 0, 0 };
		scissor.extent = { windowWidth , windowHeight };

		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

		//vkCmdDraw(commandBuffers[i], vertices.size(), 1, 0, 0);
		vkCmdDrawIndexed(commandBuffers[i], indices.size(), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		ASSERT_VK(vkEndCommandBuffer(commandBuffers[i]));
	}
}

void createSemaphores() {
	VkSemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = 0;

	ASSERT_VK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphoreImageAvailable));
	ASSERT_VK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphoreRenderingDone));
}

void startVulkan() {
#ifdef _DEBUG
	printLayerPropertiesInfo();
	printExtensionPropertiesInfo();
#endif
	createInstance();
	createGlfwWindowSurface();
#ifdef _DEBUG
	printPhysicalDevicesInfo();
#endif
	createLogicalDevice();
	initializeQueue();

	validateSurfaceSupport();

	createSwapchain();
	createImageViews();
	createRenderPass();

	createDescriptorSetLayout();

	createPipeline();
	createFrameBuffers();

	for (auto& vertex : vertices) {
		vertex.color.r = (float)(rand() % 255) / 255.0f;
		vertex.color.g = (float)(rand() % 255) / 255.0f;
		vertex.color.b = (float)(rand() % 255) / 255.0f;
	}

	createCommandPool();
	createCommandBuffers();

	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffer();
	createDescriptorPool();
	createDescriptorSet();

	recordCommandBuffers();

	createSemaphores();
}

void recreateSwapchain() {
	vkDeviceWaitIdle(device);

	// Destroy old swapchhain

	vkDestroyCommandPool(device, commandPool, nullptr);

	for (auto frameBuffer : frameBuffers) {
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
	}

	vkDestroyRenderPass(device, renderPass, nullptr);

	for (auto imageView : imageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	// Create new swapchhain

	const VkSwapchainKHR oldSwapchain = swapchain;

	createSwapchain();
	createImageViews();
	createRenderPass();
	createFrameBuffers();

	createCommandPool();
	createCommandBuffers();
	recordCommandBuffers();

	vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
}

void drawFrame() {
	uint32_t imageIndex;

	vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint32_t>::max(), semaphoreImageAvailable, VK_NULL_HANDLE, &imageIndex);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &semaphoreImageAvailable;
	std::vector<VkPipelineStageFlags> waitStateMasks = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitStateMasks.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &(commandBuffers[imageIndex]);
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphoreRenderingDone;

	ASSERT_VK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &semaphoreRenderingDone;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	ASSERT_VK(vkQueuePresentKHR(queue, &presentInfo));
}

auto gameStartTime = std::chrono::high_resolution_clock::now();
auto lastFrameTime = std::chrono::high_resolution_clock::now();

float deltaTime = 0.0f;

float maxDeltaTime = std::numeric_limits<float>::min();
float minDeletaTime = std::numeric_limits<float>::max();

void updateModelViewProj() {
	auto currentFrameTime = std::chrono::high_resolution_clock::now();

	float timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(currentFrameTime - gameStartTime).count() / 1000.0f;

	processInput(window);

	const float radius = 1.5f;
	const float speed = 1.0f;
	float camX = sin(glfwGetTime() * speed) * radius;
	float camZ = cos(glfwGetTime() * speed) * radius;

	glm::mat4 model = glm::rotate(glm::mat4(1.0f), 0 * timeSinceStart * glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//glm::mat4 view = glm::lookAt(glm::vec3(camX, 1.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 view = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
	glm::mat4 proj = glm::perspective(glm::radians(cam.fov), windowWidth / (float)windowHeight, 0.01f, 100.0f);

	proj[1][1] *= -1; // Flip Y axis

	modelViewProj = proj * view * model;

	void* rawData;
	ASSERT_VK(vkMapMemory(device, uniformBufferDeviceMemory, 0, sizeof(modelViewProj), 0, &rawData));
	memcpy(rawData, &modelViewProj, sizeof(modelViewProj));
	vkUnmapMemory(device, uniformBufferDeviceMemory);
}

void updateDeltaTime() {
	auto currentFrameTime = std::chrono::high_resolution_clock::now();

	deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentFrameTime - lastFrameTime).count() / 1000.0f;
	lastFrameTime = currentFrameTime;

	if (deltaTime > maxDeltaTime)
		maxDeltaTime = deltaTime;
	if (deltaTime < minDeletaTime)
		minDeletaTime = deltaTime;

	glfwSetWindowTitle(window, (
		std::string("Vulkan Tutorial | FOV: ") + std::to_string(cam.fov) +
		std::string(" | FPS: ") + std::to_string(1.0f / deltaTime) +
		std::string(" ( Min: ") + std::to_string(1.0f / maxDeltaTime) +
		std::string(", Max: ") + std::to_string(1.0f / minDeletaTime) + std::string(" )")
	).c_str());
}

void gameLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		drawFrame();

		updateDeltaTime();
		updateModelViewProj();
	}
}

void shutdownVulkan() {
	vkDeviceWaitIdle(device);

	vkDestroySemaphore(device, semaphoreImageAvailable, nullptr);
	vkDestroySemaphore(device, semaphoreRenderingDone, nullptr);

	//vkFreeCommandBuffers(device, commandPool, swapchainImageCount, commandBuffers.data()) Automaticly freed with the command pool

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	
	vkFreeMemory(device, uniformBufferDeviceMemory, nullptr);
	vkDestroyBuffer(device, uniformBuffer, nullptr);
	vkFreeMemory(device, vertexBufferDeviceMemory, nullptr);
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, indexBufferDeviceMemory, nullptr);
	vkDestroyBuffer(device, indexBuffer, nullptr);

	vkDestroyCommandPool(device, commandPool, nullptr);

	for (auto frameBuffer : frameBuffers) {
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
	}

	vkDestroyPipeline(device, pipeline, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

	vkDestroyShaderModule(device, shaderModuleVert, nullptr);
	vkDestroyShaderModule(device, shaderModuleFrag, nullptr);

	for (auto imageView : imageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void shutdownGlfw() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

int main() {
	srand(time(NULL));
	
	startGlfw();
	startVulkan();

	gameLoop();

	shutdownVulkan();
	shutdownGlfw();

	return 0;
}

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

uint32_t getSwapchainImageCount() {
	uint32_t swapchainImageCount;
	ASSERT_VK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr));

	return swapchainImageCount;
}

std::vector< VkPhysicalDevice> getPhysicalDevices() {
	if (physicalDevices.size() > 0) {
		return physicalDevices;
	} 

	uint32_t physicalDeviceCount;
	ASSERT_VK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
	physicalDevices.resize(physicalDeviceCount);
	ASSERT_VK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

	return physicalDevices;
}

void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
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

void printPhysicalDevicesInfo() {
	auto physicalDevices = getPhysicalDevices();

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