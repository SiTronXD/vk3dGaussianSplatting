#pragma once

#include "Buffer.h"

class UniformBuffer : public Buffer
{
public:
	void createDynamicCpuBuffer(
		const GfxAllocContext& gfxAllocContext,
		const VkDeviceSize& bufferSize
	);
};