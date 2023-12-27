#pragma once

#include "Buffer.h"

class StorageBuffer : public Buffer
{
public:
	void createStaticGpuBuffer(
		const GfxAllocContext& gfxAllocContext,
		VkDeviceSize bufferSize,
		void* data);
};