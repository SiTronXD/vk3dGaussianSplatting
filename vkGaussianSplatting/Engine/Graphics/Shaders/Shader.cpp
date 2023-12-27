#include "pch.h"

#include <fstream>

#include "Shader.h"

void Shader::readFile(const std::string& filePath, std::vector<char>& output)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	// Try to open file
	if (!file.is_open())
		Log::error("Failed to open shader file: " + filePath);

	// Allocate buffer from read position at the end of the file
	size_t fileSize = (size_t)file.tellg();
	output.resize(fileSize);

	// Read all of the file from the beginning
	file.seekg(0);
	file.read(output.data(), fileSize);

	// Close file when done
	file.close();
}

VkShaderModule Shader::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(
		this->device->getVkDevice(), 
		&createInfo, 
		nullptr, 
		&shaderModule) != VK_SUCCESS)
	{
		Log::error("Failed to create shader module.");
	}

	return shaderModule;
}

Shader::Shader()
	: shaderModule(VK_NULL_HANDLE),
	device(nullptr),
	shaderStage{}
{}

Shader::~Shader() {}

void Shader::setShaderStage(const VkPipelineShaderStageCreateInfo& shaderStage)
{
	this->shaderStage = shaderStage;
}

void Shader::loadAndCreateShaderModule(const Device& device, const std::string& filePath)
{
	this->device = &device;

	// Load file
	std::vector<char> shaderCode;
	Shader::readFile(filePath, shaderCode);

	// Create shader module
	this->shaderModule = this->createShaderModule(shaderCode);
}

void Shader::cleanup()
{
	if (this->shaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(this->device->getVkDevice(), this->shaderModule, nullptr);
	}
}
