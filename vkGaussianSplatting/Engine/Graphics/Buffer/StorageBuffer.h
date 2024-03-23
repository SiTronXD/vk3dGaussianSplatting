#pragma once

#include "Buffer.h"

class StorageBuffer : public Buffer
{
public:
	void createGpuBuffer(
		const GfxAllocContext& gfxAllocContext,
		VkDeviceSize bufferSize,
		const void* data,
		VkBufferUsageFlagBits extraFlags = (VkBufferUsageFlagBits) 0);

	inline const VkBuffer& getVkBuffer() const { return Buffer::getVkBuffer(0); }
	inline const VmaAllocation& getVmaAllocation() const { return Buffer::getVmaAllocation(0); }
};