#include "pch.h"
#include "VertexShader.h"

VertexShader::VertexShader()
{
}

VertexShader::VertexShader(const Device& device, const std::string& filePath)
{
	this->createFromFile(device, filePath);
}

VertexShader::~VertexShader()
{
}

void VertexShader::createFromFile(const Device& device, const std::string& filePath)
{
	// Load shader code and create shader module
	Shader::loadAndCreateShaderModule(device, filePath);

	// Vertex shader stage create info
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = Shader::getShaderModule();
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = nullptr; // For shader constants

	Shader::setShaderStage(vertShaderStageInfo);
}
