#pragma once

#include "CommandPool.h"
#include "CommandBuffer.h"

class CommandBufferArray
{
private:
	std::vector<CommandBuffer> commandBuffers;

public:
	CommandBufferArray();
	~CommandBufferArray();

	void createCommandBuffers(
		const Device& device,
		const CommandPool& commandPool,
		const size_t& numCommandBuffers);

	void cleanup();

	inline CommandBuffer& operator[](const uint32_t& index) { return this->commandBuffers[index]; }
};