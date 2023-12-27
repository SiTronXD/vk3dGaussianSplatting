#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class Device;

class SemaphoreArray
{
private:
	std::vector<VkSemaphore> semaphores;

	Device* device;

public:
	SemaphoreArray();
	~SemaphoreArray();

	bool create(Device& device, const uint32_t& numSemaphores);
	void cleanup();

	inline VkSemaphore& operator[](const uint32_t& index) { return this->semaphores[index]; }
};