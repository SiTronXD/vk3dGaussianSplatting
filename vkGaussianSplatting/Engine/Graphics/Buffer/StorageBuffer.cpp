#include "pch.h"
#include "StorageBuffer.h"

void StorageBuffer::createGpuBuffer(
	const GfxAllocContext& gfxAllocContext, 
	VkDeviceSize bufferSize, 
	void* data)
{
	Buffer::createGpuBuffer(
		gfxAllocContext,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		bufferSize,
		data
	);
}
