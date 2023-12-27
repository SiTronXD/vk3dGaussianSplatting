#pragma once

#include <vulkan/vulkan.h>

class Device;

class DescriptorPool
{
private:
	VkDescriptorPool descriptorPool;

	const Device* device;

public:
	DescriptorPool();
	~DescriptorPool();

	void createDescriptorPool(const Device& device, uint32_t descriptorCount);
	void createImguiDescriptorPool(const Device& device, uint32_t descriptorCount);

	void cleanup();

	inline const VkDescriptorPool& getVkDescriptorPool() const { return this->descriptorPool; }
};