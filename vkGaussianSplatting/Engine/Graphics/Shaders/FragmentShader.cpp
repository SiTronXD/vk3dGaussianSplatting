#include "pch.h"
#include "FragmentShader.h"

FragmentShader::FragmentShader()
{
}

FragmentShader::FragmentShader(
	const Device& device, 
	const std::string& filePath,
	const std::vector<SpecializationConstant>& specializationConstants)
{
	this->createFromFile(device, filePath, specializationConstants);
}

FragmentShader::~FragmentShader()
{
}

void FragmentShader::createFromFile(
	const Device& device, 
	const std::string& filePath, 
	const std::vector<SpecializationConstant>& specializationConstants)
{
	// Load shader code and create shader module
	Shader::loadAndCreateShaderModule(device, filePath, specializationConstants);

	// Fragment shader stage create info
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = Shader::getShaderModule();
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = Shader::getSpecializationInfo();

	Shader::setShaderStage(fragShaderStageInfo);
}
