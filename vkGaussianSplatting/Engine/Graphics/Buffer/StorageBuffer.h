#pragma once

#include "Buffer.h"

class StorageBuffer : public Buffer
{
public:
	void createGpuBuffer(
		const GfxAllocContext& gfxAllocContext,
		VkDeviceSize bufferSize,
		void* data);
};