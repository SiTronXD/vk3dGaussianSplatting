#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class Device;

struct SpecializationConstant
{
	void* data;
	size_t size;
};

class Shader
{
private:
	VkShaderModule shaderModule;
	VkPipelineShaderStageCreateInfo shaderStage;

	// Specialization constants
	std::vector<VkSpecializationMapEntry> specMapEntries;
	VkSpecializationInfo specializationInfo;

	const Device* device;

	void createSpecializationInfo(const std::vector<SpecializationConstant>& specializationConstants);

	VkShaderModule createShaderModule(const std::vector<char>& code);

protected:
	void readFile(const std::string& filePath, std::vector<char>& output);
	void loadAndCreateShaderModule(const Device& device, const std::string& filePath, const std::vector<SpecializationConstant>& specializationConstants);
	void setShaderStage(const VkPipelineShaderStageCreateInfo& shaderStage);

	inline const VkShaderModule& getShaderModule() const { return this->shaderModule; }
	inline const VkSpecializationInfo* getSpecializationInfo() const
	{ 
		return this->specMapEntries.size() > 0 ? 
			&this->specializationInfo :
			nullptr; 
	}

public:
	Shader();
	virtual ~Shader();

	void cleanup();

	inline const VkPipelineShaderStageCreateInfo& getShaderStage() { return this->shaderStage; }
};