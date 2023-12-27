#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class Device;

class Shader
{
private:
	VkShaderModule shaderModule;
	VkPipelineShaderStageCreateInfo shaderStage;

	const Device* device;

	VkShaderModule createShaderModule(const std::vector<char>& code);

protected:
	void readFile(const std::string& filePath, std::vector<char>& output);
	void loadAndCreateShaderModule(const Device& device, const std::string& filePath);
	void setShaderStage(const VkPipelineShaderStageCreateInfo& shaderStage);

	inline const VkShaderModule& getShaderModule() { return this->shaderModule; }

public:
	Shader();
	virtual ~Shader();

	void cleanup();

	inline const VkPipelineShaderStageCreateInfo& getShaderStage() { return this->shaderStage; }
};