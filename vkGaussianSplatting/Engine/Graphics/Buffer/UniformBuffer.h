#pragma once

#include "Buffer.h"

class UniformBuffer : public Buffer
{
public:
	void createCpuGpuBuffer(
		const GfxAllocContext& gfxAllocContext,
		const VkDeviceSize& bufferSize
	);
};