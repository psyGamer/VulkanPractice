#include "Pipeline.h"

Pipeline::Pipeline() { }

void Pipeline::Initialize(Shader& vertexShader, Shader& fragmentShader, uint32_t width, uint32_t height) {

	m_ShaderStageCreateInfoVertex.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_ShaderStageCreateInfoVertex.pNext = nullptr;
	m_ShaderStageCreateInfoVertex.flags = 0;
	m_ShaderStageCreateInfoVertex.stage = VK_SHADER_STAGE_VERTEX_BIT;
	m_ShaderStageCreateInfoVertex.module = vertexShader.GetShaderModule();
	m_ShaderStageCreateInfoVertex.pName = "main";
	m_ShaderStageCreateInfoVertex.pSpecializationInfo = nullptr;

	m_ShaderStageCreateInfoFragment.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_ShaderStageCreateInfoFragment.pNext = nullptr;
	m_ShaderStageCreateInfoFragment.flags = 0;
	m_ShaderStageCreateInfoFragment.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	m_ShaderStageCreateInfoFragment.module = fragmentShader.GetShaderModule();
	m_ShaderStageCreateInfoFragment.pName = "main";
	m_ShaderStageCreateInfoFragment.pSpecializationInfo = nullptr;

	m_VertexBindingDescription = Vertex::getBindingDescription();
	m_VertexAttributeDescriptions = Vertex::getAttributeDescriptions();

	m_VertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_VertexInputCreateInfo.pNext = nullptr;
	m_VertexInputCreateInfo.flags = 0;
	m_VertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	m_VertexInputCreateInfo.pVertexBindingDescriptions = &m_VertexBindingDescription;
	m_VertexInputCreateInfo.vertexAttributeDescriptionCount = m_VertexAttributeDescriptions.size();
	m_VertexInputCreateInfo.pVertexAttributeDescriptions = m_VertexAttributeDescriptions.data();

	m_InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_InputAssemblyCreateInfo.pNext = nullptr;
	m_InputAssemblyCreateInfo.flags = 0;
	m_InputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m_InputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	m_Viewport.x = 0.0f;
	m_Viewport.y = 0.0f;
	m_Viewport.width = width;
	m_Viewport.height = height;
	m_Viewport.minDepth = 0.0f;
	m_Viewport.maxDepth = 1.0f;

	m_Scissor.offset = { 0, 0 };
	m_Scissor.extent = { width, height };

	m_ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_ViewportStateCreateInfo.pNext = nullptr;
	m_ViewportStateCreateInfo.flags = 0;
	m_ViewportStateCreateInfo.viewportCount = 1;
	m_ViewportStateCreateInfo.pViewports = &m_Viewport;
	m_ViewportStateCreateInfo.scissorCount = 1;
	m_ViewportStateCreateInfo.pScissors = &m_Scissor;

	m_RasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_RasterizationCreateInfo.pNext = nullptr;
	m_RasterizationCreateInfo.flags = 0;
	m_RasterizationCreateInfo.depthClampEnable = VK_FALSE;
	m_RasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	m_RasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
#ifdef _DEBUG
	m_RasterizationCreateInfo.cullMode = VK_CULL_MODE_NONE;
#else
	m_RasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
#endif
	m_RasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	m_RasterizationCreateInfo.depthBiasEnable = VK_FALSE;
	m_RasterizationCreateInfo.depthBiasConstantFactor = VK_FALSE;
	m_RasterizationCreateInfo.depthBiasClamp = 0.0f;
	m_RasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;
	m_RasterizationCreateInfo.lineWidth = 1.0f;

	m_MultisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_MultisampleCreateInfo.pNext = nullptr;
	m_MultisampleCreateInfo.flags = 0;
	m_MultisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_MultisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	m_MultisampleCreateInfo.minSampleShading = 1.0f;
	m_MultisampleCreateInfo.pSampleMask = nullptr;
	m_MultisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	m_MultisampleCreateInfo.alphaToOneEnable = VK_FALSE;

	m_ColorBlendAttachmentState.blendEnable = VK_TRUE;
	m_ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	m_ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	m_ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	m_ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	m_ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	m_ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	m_ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	m_ColorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_ColorBlendCreateInfo.pNext = nullptr;
	m_ColorBlendCreateInfo.flags = 0;
	m_ColorBlendCreateInfo.logicOpEnable = VK_FALSE;
	m_ColorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
	m_ColorBlendCreateInfo.attachmentCount = 1;
	m_ColorBlendCreateInfo.pAttachments = &m_ColorBlendAttachmentState;
	m_ColorBlendCreateInfo.blendConstants[0] = 0.0f;
	m_ColorBlendCreateInfo.blendConstants[1] = 0.0f;
	m_ColorBlendCreateInfo.blendConstants[2] = 0.0f;
	m_ColorBlendCreateInfo.blendConstants[3] = 0.0f;

	m_DynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	m_DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	m_DynamicStateCreateInfo.pNext = nullptr;
	m_DynamicStateCreateInfo.flags = 0;
	m_DynamicStateCreateInfo.dynamicStateCount = m_DynamicStates.size();
	m_DynamicStateCreateInfo.pDynamicStates = m_DynamicStates.data();

	m_DepthStencilState = DepthImage::GetDepthStencilCreateInfoOpaque();

	m_PushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	m_PushConstantRange.offset = 0;
	m_PushConstantRange.size = sizeof(uint32_t);

	m_Initialized = true;
}

void Pipeline::Create(VkDevice& device, VkRenderPass& renderPass, VkDescriptorSetLayout& descriptorSetLayout) {
	if (!m_Initialized) {
		std::cerr << "This pipeline isn't initialized yet!";
		throw std::logic_error("This pipeline isn't initialized yet!");
	}
	if (m_Created) {
		std::cerr << "This pipeline was alreaddy created!";
		throw std::logic_error("This pipeline was alreaddy created!");
	}

	m_Device = device;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStates = { 
		m_ShaderStageCreateInfoVertex, m_ShaderStageCreateInfoFragment 
	};

	VkPipelineLayoutCreateInfo layoutCreateInfo;
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pNext = nullptr;
	layoutCreateInfo.flags = 0;
	layoutCreateInfo.setLayoutCount = 1;
	layoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	layoutCreateInfo.pushConstantRangeCount = 1;
	layoutCreateInfo.pPushConstantRanges = &m_PushConstantRange;

	ASSERT_VK(vkCreatePipelineLayout(m_Device, &layoutCreateInfo, nullptr, &m_PipelineLayout));

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stageCount = shaderStates.size();
	pipelineCreateInfo.pStages = shaderStates.data();
	pipelineCreateInfo.pVertexInputState = &m_VertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &m_InputAssemblyCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &m_ViewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &m_RasterizationCreateInfo;
	pipelineCreateInfo.pMultisampleState = &m_MultisampleCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &m_DepthStencilState;
	pipelineCreateInfo.pColorBlendState = &m_ColorBlendCreateInfo;
	pipelineCreateInfo.pDynamicState = &m_DynamicStateCreateInfo;
	pipelineCreateInfo.layout = m_PipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	ASSERT_VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_Pipeline));

	m_Created = true;
}

void Pipeline::Destory() {
	if (m_Created) {
		vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);

		m_Created = false;
	}
}

void Pipeline::SetPolygonMode(VkPolygonMode polygonMode) {
	m_RasterizationCreateInfo.polygonMode = polygonMode;
}

VkPipeline Pipeline::GetPipeline() const {
	if (!m_Created) {
		std::cerr << "This pipeline isn't created yet!";
		throw std::logic_error("This pipeline isn't created yet!");
	}

	return m_Pipeline;
}
VkPipelineLayout Pipeline::GetLayout() const {
	if (!m_Created) {
		std::cerr << "This pipeline isn't created yet!";
		throw std::logic_error("This pipeline isn't created yet!");
	}

	return m_PipelineLayout;
}