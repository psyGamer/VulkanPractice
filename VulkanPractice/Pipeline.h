#pragma once

#include <vector>

#include "Image/DepthImage.h"
#include "Vertex.h"

#include "VulkanUtils.h"

class Pipeline {

private:
	bool m_Initialized = false, m_Created = false;

	VkDevice m_Device = VK_NULL_HANDLE;

	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

	VkPipelineShaderStageCreateInfo m_ShaderStageCreateInfoVertex;
	VkPipelineShaderStageCreateInfo m_ShaderStageCreateInfoFragment;

	VkPipelineVertexInputStateCreateInfo m_VertexInputCreateInfo;
	VkPipelineInputAssemblyStateCreateInfo m_InputAssemblyCreateInfo;
	
	VkVertexInputBindingDescription m_VertexBindingDescription;
	std::vector<VkVertexInputAttributeDescription> m_VertexAttributeDescriptions;

	VkViewport m_Viewport;
	VkRect2D m_Scissor;

	VkPipelineViewportStateCreateInfo m_ViewportStateCreateInfo;
	VkPipelineRasterizationStateCreateInfo m_RasterizationCreateInfo;
	VkPipelineMultisampleStateCreateInfo m_MultisampleCreateInfo;
	VkPipelineColorBlendAttachmentState m_ColorBlendAttachmentState;
	VkPipelineColorBlendStateCreateInfo m_ColorBlendCreateInfo;

	VkPipelineDepthStencilStateCreateInfo m_DepthStencilState;

	std::vector<VkDynamicState> m_DynamicStates;
	VkPipelineDynamicStateCreateInfo m_DynamicStateCreateInfo;
	VkPushConstantRange m_PushConstantRange;

public:
	Pipeline();

	void Initialize(VkShaderModule& vertexShader, VkShaderModule& fragmentShader, uint32_t width, uint32_t height);
	void Create(VkDevice& device, VkRenderPass& renderPass, VkDescriptorSetLayout& descriptorSetLayout);
	void Destory();

	VkPipeline GetPipeline();
	VkPipelineLayout GetLayout();
};