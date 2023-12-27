#include "pch.h"
#include "CommandPool.h"

CommandPool::CommandPool()
	: commandPool(VK_NULL_HANDLE),
	device(nullptr)
{
}

CommandPool::~CommandPool()
{
}

void CommandPool::create(
	const Device& device, 
	QueueFamilies& queueFamilies, 
	VkCommandPoolCreateFlags flags)
{
	this->device = &device;

	const QueueFamilyIndices& queueFamilyIndices = queueFamilies.getIndices();

	// Create command pool for graphics queue
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = flags;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	if (vkCreateCommandPool(
		this->device->getVkDevice(),
		&poolInfo,
		nullptr,
		&this->commandPool) != VK_SUCCESS)
	{
		Log::error("Failed to create command pool.");
	}
}

void CommandPool::cleanup()
{
	// Destroys command pool and command buffers allocated from it
	vkDestroyCommandPool(this->device->getVkDevice(), this->commandPool, nullptr);
}