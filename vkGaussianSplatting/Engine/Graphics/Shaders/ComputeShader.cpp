#include "pch.h"
#include "ComputeShader.h"

ComputeShader::ComputeShader()
{
}

ComputeShader::ComputeShader(const Device& device, const std::string& filePath)
{
	this->createFromFile(device, filePath);
}

ComputeShader::~ComputeShader()
{
}

void ComputeShader::createFromFile(const Device& device, const std::string& filePath)
{
	// Load shader code and create shader module
	Shader::loadAndCreateShaderModule(device, filePath);

	// Vertex shader stage create info
	VkPipelineShaderStageCreateInfo compShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	compShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	compShaderStageInfo.module = Shader::getShaderModule();
	compShaderStageInfo.pName = "main";
	compShaderStageInfo.pSpecializationInfo = nullptr; // For shader constants

	Shader::setShaderStage(compShaderStageInfo);
}
