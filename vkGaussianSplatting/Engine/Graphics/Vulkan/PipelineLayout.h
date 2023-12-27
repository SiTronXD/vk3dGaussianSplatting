#pragma once

#include "DescriptorSetLayout.h"

class Device;

class PipelineLayout
{
private:
	VkPipelineLayout pipelineLayout;

	DescriptorSetLayout descriptorSetLayout;

	VkShaderStageFlags pushConstantStage;
	uint32_t pushConstantSize;

	const Device* device;

public:
	PipelineLayout();
	~PipelineLayout();

	void createPipelineLayout(
		const Device& device, 
		const std::vector<DescriptorSetLayoutElement>& descriptorSetLayoutElements,
		const VkShaderStageFlags& pushConstantShaderStageFlags = VK_SHADER_STAGE_ALL,
		const uint32_t& pushConstantSize = 0);
	void createImguiPipelineLayout(const Device& device);

	void cleanup();

	inline const VkPipelineLayout& getVkPipelineLayout() const { return this->pipelineLayout; }
	inline VkShaderStageFlags getPushConstantStage() const { return this->pushConstantStage; }
	inline uint32_t getPushConstantSize() const { return this->pushConstantSize; }
};