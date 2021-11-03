#pragma once

#include "Vulkan.h"

class Shader {

private:
	bool m_Loaded = false;

	VkDevice m_Device;
	VkShaderModule m_ShaderModule = VK_NULL_HANDLE;

public:
	Shader();

	Shader(const VkDevice& device);
	Shader(const VkDevice& device, const std::string& filePath);

	void Load(const std::string& filePath);
	void Destory();

	VkShaderModule GetShaderModule() const;
};
