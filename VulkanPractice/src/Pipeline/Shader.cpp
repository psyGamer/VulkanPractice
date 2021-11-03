#include "Shader.h"

#include "Util/FileUtil.h"

Shader::Shader()
	: m_Device(VK_NULL_HANDLE) { }

Shader::Shader(const VkDevice& pDevice) 
	: m_Device(pDevice) { }
Shader::Shader(const VkDevice& pDevice, const std::string& filePath)
	: m_Device(pDevice) {

	Load(filePath);
}

void Shader::Load(const std::string& filePath) {
	ASSERT(m_Loaded, "This shader was already loaded!");

	auto shaderCode = FileUtil::ReadFile(filePath);

	VkShaderModuleCreateInfo shaderCreateInfo;
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.pNext = nullptr;
	shaderCreateInfo.flags = 0;
	shaderCreateInfo.codeSize = shaderCode.size();
	shaderCreateInfo.pCode = reinterpret_cast<uint32_t*>(shaderCode.data());

	ASSERT_VK(vkCreateShaderModule(m_Device, &shaderCreateInfo, nullptr, &m_ShaderModule));

	m_Loaded = true;
}

void Shader::Destory() {
	if (!m_Loaded)
		return;

	vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);

	m_ShaderModule = VK_NULL_HANDLE;
	m_Loaded = false;
}

VkShaderModule Shader::GetShaderModule() const {
	ASSERT(!m_Loaded, "This shader wasn't loaded yet!");

	return m_ShaderModule;
}
