#pragma once

#include "Buffer.h"

class StagingBuffer : public Buffer
{
public:
	void createStagingBuffer(
		const GfxAllocContext& gfxAllocContext,
		const VkDeviceSize& size);

	inline const VkBuffer& getVkBuffer() const { return Buffer::getVkBuffer(0); }
	inline const VmaAllocation& getVmaAllocation() const { return Buffer::getVmaAllocation(0); }
};