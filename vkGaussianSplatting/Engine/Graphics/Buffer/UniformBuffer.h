#pragma once

#include "Buffer.h"

class UniformBuffer : public Buffer
{
public:
	void createCpuGpuBuffer(
		const GfxAllocContext& gfxAllocContext,
		const VkDeviceSize& bufferSize
	);

	inline const VkBuffer& getVkBuffer(uint32_t index) const { return Buffer::getVkBuffer(index); }
	inline const VmaAllocation& getVmaAllocation(uint32_t index) const { return Buffer::getVmaAllocation(index); }
};