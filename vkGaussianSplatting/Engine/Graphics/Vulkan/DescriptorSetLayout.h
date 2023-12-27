#pragma once

#include <vulkan/vulkan.h>

class Device;

struct DescriptorSetLayoutElement
{
	VkDescriptorType descriptorType;
	VkShaderStageFlags stage;
};

class DescriptorSetLayout
{
private:
	VkDescriptorSetLayout descriptorSetLayout;

	const Device* device;

public:
	DescriptorSetLayout();
	~DescriptorSetLayout();

	void createDescriptorSetLayout(
		const Device& device, 
		const std::vector<DescriptorSetLayoutElement>& elements);
	void createImguiDescriptorSetLayout(const Device& device);

	void cleanup();

	inline const VkDescriptorSetLayout& getVkDescriptorSetLayout() const 
		{ return this->descriptorSetLayout; }
};