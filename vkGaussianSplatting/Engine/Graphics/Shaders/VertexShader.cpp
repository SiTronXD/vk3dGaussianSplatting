#include "pch.h"
#include "VertexShader.h"

VertexShader::VertexShader()
{
}

VertexShader::VertexShader(
	const Device& device, 
	const std::string& filePath,
	const std::vector<SpecializationConstant>& specializationConstants)
{
	this->createFromFile(device, filePath, specializationConstants);
}

VertexShader::~VertexShader()
{
}

void VertexShader::createFromFile(
	const Device& device, 
	const std::string& filePath,
	const std::vector<SpecializationConstant>& specializationConstants)
{
	// Load shader code and create shader module
	Shader::loadAndCreateShaderModule(device, filePath, specializationConstants);

	// Vertex shader stage create info
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = Shader::getShaderModule();
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = Shader::getSpecializationInfo();

	Shader::setShaderStage(vertShaderStageInfo);
}
