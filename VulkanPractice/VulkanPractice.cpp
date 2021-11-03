#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include <stdlib.h>
#include <time.h>

#include "VulkanUtils.h"
#include "Image/DepthImage.h"
#include "Image/Image.h"

#include "Pipeline.h"
#include "Camera.h"
#include "Vertex.h"
#include "MeshHelper.h"
#include "Mesh.h"

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;

	glm::vec3 lightPosition;
};

void recreateSwapchain();

Camera camera;
DepthImage depthImage;

Pipeline pipeline;
Pipeline pipelineWireframe;

Image diamondImage;
Image brickImage;
Image brickNormalImage;
Mesh dragonMesh;

VkInstance instance;
VkSurfaceKHR surface;
VkDevice device;
VkQueue queue;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;

std::vector<VkImageView> imageViews;
std::vector<VkFramebuffer> frameBuffers;
std::vector<VkCommandBuffer> commandBuffers;

VkShaderModule shaderModuleVert;
VkShaderModule shaderModuleFrag;

VkRenderPass renderPass;

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

UniformBufferObject ubo;

VkSemaphore semaphoreImageAvailable;
VkSemaphore semaphoreRenderingDone;

GLFWwindow* window;

uint32_t windowWidth = 800, windowHeight = 600;

float deltaTime = 0.0f;
bool wireframe = false;

const VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM;

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

void onWindowResized(GLFWwindow * window, int width, int height) {
	if (width == 0 || height == 0)
		return;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(getPhysicalDevices(instance)[0], surface, &surfaceCapabilities);

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
		lastMouseX = -xpos;
		lastMouseY =  ypos;
		firstMouse = false;
	}

	float xOffset = (xpos - lastMouseX) * sensitivity;
	float yOffset = (ypos - lastMouseY) * sensitivity;

	lastMouseX = xpos;
	lastMouseY = ypos;

	camera.Rotate({ xOffset, yOffset });

	if (camera.GetPitch() > 89.0f)
		camera.SetPitch(89.0f);
	if (camera.GetPitch() < -89.0f)
		camera.SetPitch(-89.0f);

	glm::vec3 direction;
	direction.x = cos(glm::radians(camera.GetYaw())) * cos(glm::radians(camera.GetPitch()));
	direction.y = sin(glm::radians(camera.GetPitch()));
	direction.z = sin(glm::radians(camera.GetYaw())) * cos(glm::radians(camera.GetPitch()));

	camera.SetFront(glm::normalize(direction));
}

void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
	const float sensitivity = 2.0f;

	camera.ChangeFOV(-yoffset * sensitivity);

	if (camera.GetFOV() < 1.0f)
		camera.SetFOV(1.0f);
	if (camera.GetFOV() > 150.0f)
		camera.SetFOV(150.0f);
}

void createCommandBuffers();
void recordCommandBuffers();

bool zDown = false;

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	const float cameraSpeed = (
		glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? 2.0f : 
		glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ? 0.5f : 1.0f
	) * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.Move(+cameraSpeed * camera.GetFront());
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.Move(-cameraSpeed * camera.GetFront());
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.Move(-glm::normalize(glm::cross(camera.GetFront(), camera.GetUp())) * cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.Move(+glm::normalize(glm::cross(camera.GetFront(), camera.GetUp())) * cameraSpeed);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.Move(-glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.Move(+glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed);

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && !zDown) {
		wireframe = !wireframe;

		createCommandBuffers();
		recordCommandBuffers();

		zDown = true;
	} else if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE && zDown) {
		zDown = false;
	}
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
	auto physicalDevices = getPhysicalDevices(instance);

	float queuePriorities[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	VkDeviceQueueCreateInfo deviceQueueCreateInfo;
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.pNext = nullptr;
	deviceQueueCreateInfo.flags = 0;
	deviceQueueCreateInfo.queueFamilyIndex = 0; //TODO select "best" queue family
	deviceQueueCreateInfo.queueCount = 1; //TODO civ (check if valid)
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

	VkPhysicalDeviceFeatures usedFeatures{ };
	usedFeatures.samplerAnisotropy = VK_TRUE;
	usedFeatures.fillModeNonSolid = VK_TRUE;

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
	auto physicalDevices = getPhysicalDevices(instance);

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
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(getPhysicalDevices(instance)[0], surface, &surfaceCapabilities);
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
	uint32_t swapchainImageCount = getSwapchainImageCount(device, swapchain);

	std::vector<VkImage> swapchainImages;
	swapchainImages.resize(swapchainImageCount);
	ASSERT_VK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data()));

	imageViews.resize(swapchainImageCount);

	for (int i = 0; i < swapchainImageCount; i++) {
		createImageView(device, swapchainImages[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, imageViews[i]);
	}
}

void createRenderPass() {
	std::vector<VkAttachmentDescription> attachmentDescriptions;

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
	attachmentDescriptions.push_back(attachmentDescription);

	VkAttachmentReference attachmentReference;
	attachmentReference.attachment = 0;
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = DepthImage::GetDepthAttachmentDescription(getPhysicalDevices(instance)[0]);
	attachmentDescriptions.push_back(depthAttachment);

	VkAttachmentReference depthAttachmentReference;
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &attachmentReference;
	subpassDescription.pResolveAttachments = nullptr;
	subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
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
	renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
	renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;

	ASSERT_VK(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass));
}

void createDescriptorSetLayout() {
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

	VkDescriptorSetLayoutBinding modelViewProjDescriptorSetLayoutBinding;
	modelViewProjDescriptorSetLayoutBinding.binding = 0;
	modelViewProjDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	modelViewProjDescriptorSetLayoutBinding.descriptorCount = 1;
	modelViewProjDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	modelViewProjDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings.push_back(modelViewProjDescriptorSetLayoutBinding);

	VkDescriptorSetLayoutBinding samplerDescriptorSetLayoutBinding;
	samplerDescriptorSetLayoutBinding.binding = 1;
	samplerDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerDescriptorSetLayoutBinding.descriptorCount = 1;
	samplerDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings.push_back(samplerDescriptorSetLayoutBinding);

	VkDescriptorSetLayoutBinding samplerNormalDescriptorSetLayoutBinding;
	samplerNormalDescriptorSetLayoutBinding.binding = 2;
	samplerNormalDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerNormalDescriptorSetLayoutBinding.descriptorCount = 1;
	samplerNormalDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerNormalDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings.push_back(samplerNormalDescriptorSetLayoutBinding);

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = nullptr;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBindings.size();
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	ASSERT_VK(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));
}

void createPipeline() {
	auto shaderCodeVert = readFile("Shaders/vert.spv");
	auto shaderCodeFrag = readFile("Shaders/frag.spv");

	createShaderModule(shaderCodeVert, &shaderModuleVert, device);
	createShaderModule(shaderCodeFrag, &shaderModuleFrag, device);

	pipeline.Initialize(shaderModuleVert, shaderModuleFrag, windowWidth, windowHeight);
	pipeline.Create(device, renderPass, descriptorSetLayout);

	pipelineWireframe.Initialize(shaderModuleVert, shaderModuleFrag, windowWidth, windowHeight);
	pipelineWireframe.SetPolygonMode(VK_POLYGON_MODE_LINE);
	pipelineWireframe.Create(device, renderPass, descriptorSetLayout);
}

void createFrameBuffers() {
	uint32_t swapchainImageCount = getSwapchainImageCount(device, swapchain);

	frameBuffers.resize(swapchainImageCount);

	for (int i = 0; i < swapchainImageCount; i++) {
		std::vector<VkImageView> attachmentViews;
		attachmentViews.push_back(imageViews[i]);
		attachmentViews.push_back(depthImage.GetImageView());

		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = attachmentViews.size();
		framebufferCreateInfo.pAttachments = attachmentViews.data();
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

void createDepthImage() {
	depthImage.Create(device, getPhysicalDevices(instance)[0], commandPool, queue, windowWidth, windowHeight);
}

void createCommandBuffers() {
	uint32_t swapchainImageCount = getSwapchainImageCount(device, swapchain);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = swapchainImageCount;

	commandBuffers.resize(swapchainImageCount);

	ASSERT_VK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data()));
}

void loadTexture() {
	diamondImage.LoadImage("Assets/diamond_block.png");
	diamondImage.Upload(device, getPhysicalDevices(instance)[0], commandPool, queue);

	brickImage.LoadImage("Assets/151.jpg");
	brickImage.Upload(device, getPhysicalDevices(instance)[0], commandPool, queue);
	brickNormalImage.LoadImage("Assets/151_norm.jpg");
	brickNormalImage.Upload(device, getPhysicalDevices(instance)[0], commandPool, queue);
}

void loadMesh() {
	/*
	dragonMesh.Create("Assets/dragon.obj");

	vertices = dragonMesh.GetVertices();
	indices = dragonMesh.GetIndices();
	*/

	vertices = getHorizontalQuadVertices();
	indices = getQuadIndices();
}

void createVertexBuffer() {
	createAndUploadBuffer(device, getPhysicalDevices(instance)[0], queue, commandPool, vertices, vertexBuffer, vertexBufferDeviceMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

void createIndexBuffer() {
	createAndUploadBuffer(device, getPhysicalDevices(instance)[0], queue, commandPool, indices, indexBuffer, indexBufferDeviceMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void createUniformBuffer() {
	VkDeviceSize bufferSize = sizeof(ubo);

	createBuffer(device, getPhysicalDevices(instance)[0], uniformBuffer, uniformBufferDeviceMemory, bufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

void createDescriptorPool() {
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes;

	VkDescriptorPoolSize modelViewProjDescriptorPoolSize;
	modelViewProjDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	modelViewProjDescriptorPoolSize.descriptorCount = 1;
	descriptorPoolSizes.push_back(modelViewProjDescriptorPoolSize);

	VkDescriptorPoolSize samplerDescriptorPoolSize;
	samplerDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerDescriptorPoolSize.descriptorCount = 2;
	descriptorPoolSizes.push_back(samplerDescriptorPoolSize);

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
	
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
	descriptorBufferInfo.range = sizeof(ubo);

	std::vector<VkDescriptorImageInfo> descriptorImageInfos;

	VkDescriptorImageInfo descriptorImageInfo;
	descriptorImageInfo.sampler = brickImage.GetSampler();
	descriptorImageInfo.imageView = brickImage.GetImageView();
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfos.push_back(descriptorImageInfo);
	VkDescriptorImageInfo descriptorImageNormalInfo;
	descriptorImageNormalInfo.sampler = brickNormalImage.GetSampler();
	descriptorImageNormalInfo.imageView = brickNormalImage.GetImageView();
	descriptorImageNormalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfos.push_back(descriptorImageNormalInfo);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

	VkWriteDescriptorSet modelViewProjWriteDescriptorSet;
	modelViewProjWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	modelViewProjWriteDescriptorSet.pNext = nullptr;
	modelViewProjWriteDescriptorSet.dstSet = descriptorSet;
	modelViewProjWriteDescriptorSet.dstBinding = 0;
	modelViewProjWriteDescriptorSet.dstArrayElement = 0;
	modelViewProjWriteDescriptorSet.descriptorCount = 1;
	modelViewProjWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	modelViewProjWriteDescriptorSet.pImageInfo = nullptr;
	modelViewProjWriteDescriptorSet.pBufferInfo = &descriptorBufferInfo;
	modelViewProjWriteDescriptorSet.pTexelBufferView = nullptr;
	writeDescriptorSets.push_back(modelViewProjWriteDescriptorSet);

	VkWriteDescriptorSet samplerWriteDescriptorSet;
	samplerWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	samplerWriteDescriptorSet.pNext = nullptr;
	samplerWriteDescriptorSet.dstSet = descriptorSet;
	samplerWriteDescriptorSet.dstBinding = 1;
	samplerWriteDescriptorSet.dstArrayElement = 0;
	samplerWriteDescriptorSet.descriptorCount = descriptorImageInfos.size();
	samplerWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerWriteDescriptorSet.pImageInfo = descriptorImageInfos.data();
	samplerWriteDescriptorSet.pBufferInfo = nullptr;
	samplerWriteDescriptorSet.pTexelBufferView = nullptr;
	writeDescriptorSets.push_back(samplerWriteDescriptorSet);

	vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

void recordCommandBuffers() {
	uint32_t swapchainImageCount = getSwapchainImageCount(device, swapchain);

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	for (int i = 0; i < swapchainImageCount; i++) {
		ASSERT_VK(vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo));
		
		std::vector<VkClearValue> clearValues;
		clearValues.push_back({ 0.0f, 0.0f, 0.0f, 1.0f }); // Clear Value
		clearValues.push_back({ 1.0f, 0.0f }); // Depth Clear Value

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = frameBuffers[i];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = { windowWidth, windowHeight };
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		uint32_t shadingMode = wireframe ? 3 : 1; // 1: Phong 2: Cartoon 3: Flag
		const Pipeline& pipelineToUse = wireframe ? pipelineWireframe : pipeline;

		vkCmdPushConstants(commandBuffers[i], pipelineToUse.GetLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(shadingMode), &shadingMode);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineToUse.GetPipeline());

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

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineToUse.GetLayout(), 0, 1, &descriptorSet, 0, nullptr);

		//vkCmdDraw(commandBuffers[i], vertices.size(), 1, 0, 0);
		vkCmdDrawIndexed(commandBuffers[i], indices.size(), 1, 0, 0, 0);
		/*
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineWireframe.GetPipeline());

		viewport.x = windowWidth / 2.0f;
		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
		usePhong = VK_FALSE;
		vkCmdPushConstants(commandBuffers[i], pipelineWireframe.GetLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(usePhong), &usePhong);

		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetLayout(), 0, 1, &descriptorSet, 0, nullptr);
		vkCmdDrawIndexed(commandBuffers[i], indices.size(), 1, 0, 0, 0);
		*/
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
	printPhysicalDevicesInfo(instance, surface);
#endif
	createLogicalDevice();
	initializeQueue();

	validateSurfaceSupport();

	createSwapchain();
	createImageViews();
	createRenderPass();

	createDescriptorSetLayout();

	createPipeline();
	createCommandPool();
	createDepthImage();
	createFrameBuffers();

	for (auto& vertex : vertices) {
		vertex.color.r = (float)(rand() % 255) / 255.0f;
		vertex.color.g = (float)(rand() % 255) / 255.0f;
		vertex.color.b = (float)(rand() % 255) / 255.0f;
	}
	
	createCommandBuffers();

	loadTexture();
	loadMesh();
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

	depthImage.Destory();

	for (auto frameBuffer : frameBuffers) {
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
	}

	vkDestroyRenderPass(device, renderPass, nullptr);

	for (auto imageView : imageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	// Create new swapchhain

	const VkSwapchainKHR oldSwapchain = swapchain;

	createDepthImage();

	createSwapchain();
	createImageViews();
	createRenderPass();
	createFrameBuffers();

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

auto lastFrameTime = std::chrono::high_resolution_clock::now();

float maxDeltaTime = std::numeric_limits<float>::min();
float minDeletaTime = std::numeric_limits<float>::max();

void updateModelViewProj() {
	static auto gameStartTime = std::chrono::high_resolution_clock::now();
	auto currentFrameTime = std::chrono::high_resolution_clock::now();

	float timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(currentFrameTime - gameStartTime).count() / 1000.0f;

	processInput(window);

	ubo.model = glm::mat4(1.0f);
	ubo.model = glm::translate(ubo.model, glm::vec3(0.0f, 0.5f, 0.0f));
	//ubo.model = glm::scale(ubo.model, glm::vec3(0.1f, 0.1f, 0.1f));
	ubo.model = glm::rotate(ubo.model, timeSinceStart * glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	ubo.view = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetFront(), camera.GetUp());
	ubo.proj = glm::perspective(glm::radians(camera.GetFOV()), windowWidth / (float)windowHeight, 0.01f, 100.0f);
	ubo.proj[1][1] *= -1; // Flip Y axis

	ubo.lightPosition = glm::rotate(glm::mat4(1.0f), 0.0f * timeSinceStart * glm::radians(90.0f), glm::vec3(1.0, 0.0f, 1.0f)) * glm::vec4(0.0f, -1.0f, 3.0f, 0.0f);

	void* data;
	ASSERT_VK(vkMapMemory(device, uniformBufferDeviceMemory, 0, sizeof(ubo), 0, &data));
	memcpy(data, &ubo, sizeof(ubo));
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
		std::string("Vulkan Tutorial | FOV: ") + std::to_string(camera.GetFOV()) +
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

	diamondImage.Destory();
	brickImage.Destory();
	brickNormalImage.Destory();

	for (auto frameBuffer : frameBuffers) {
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
	}

	depthImage.Destory();

	vkDestroyCommandPool(device, commandPool, nullptr);

	pipeline.Destory();
	pipelineWireframe.Destory();

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

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
