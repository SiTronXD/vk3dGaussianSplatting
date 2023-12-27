#include "pch.h"
#include "StorageBuffer.h"

void StorageBuffer::createStaticGpuBuffer(
	const GfxAllocContext& gfxAllocContext, 
	VkDeviceSize bufferSize, 
	void* data)
{
	Buffer::createStaticGpuBuffer(
		gfxAllocContext,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		bufferSize,
		data
	);
}
