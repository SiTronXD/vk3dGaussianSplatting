#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class Device;

class FenceArray
{
private:
	std::vector<VkFence> fences;

	Device* device;

public:
	FenceArray();
	~FenceArray();

	bool create(Device& device, const uint32_t& numFences, const VkFenceCreateFlags& flags);

	void reset(const uint32_t& index);
	void cleanup();

	inline VkFence& operator[](const uint32_t& index) { return this->fences[index]; }
};

