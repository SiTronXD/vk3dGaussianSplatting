#include "pch.h"
#include "CommandBufferArray.h"

CommandBufferArray::CommandBufferArray()
{
}

CommandBufferArray::~CommandBufferArray()
{
	this->cleanup();
}

void CommandBufferArray::createCommandBuffers(
	const Device& device,
	const CommandPool& commandPool,
	const size_t& numCommandBuffers)
{
	this->commandBuffers.resize(numCommandBuffers);
	std::vector<VkCommandBuffer> commandBufferData(numCommandBuffers);

	// Allocate command buffer from command pool
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool.getVkCommandPool();
	allocInfo.commandBufferCount = (uint32_t) numCommandBuffers;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	if (vkAllocateCommandBuffers(
		device.getVkDevice(),
		&allocInfo,
		commandBufferData.data()) != VK_SUCCESS)
	{
		Log::error("Failed to allocate command buffers.");
	}

	// Populate command buffers
	for (size_t i = 0; i < commandBufferData.size(); ++i)
	{
		this->commandBuffers[i].setVkCommandBuffer(commandBufferData[i]);
		this->commandBuffers[i].init(device);
	}
}

void CommandBufferArray::cleanup()
{
	this->commandBuffers.clear();
}
