#pragma once

#include <vulkan/vulkan.h>

class Device;
class QueueFamilies;

class CommandPool
{
private:
	VkCommandPool commandPool;

	const Device* device;

public:
	CommandPool();
	~CommandPool();

	void create(
		const Device& device,
		QueueFamilies& queueFamilies,
		VkCommandPoolCreateFlags flags);
	void cleanup();

	inline const VkCommandPool& getVkCommandPool() const { return this->commandPool; }
};