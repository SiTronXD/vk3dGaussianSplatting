#include "pch.h"
#include "StorageBuffer.h"

void StorageBuffer::createGpuBuffer(
	const GfxAllocContext& gfxAllocContext, 
	VkDeviceSize bufferSize, 
	const void* data,
	VkBufferUsageFlagBits extraFlags)
{
	Buffer::createGpuBuffer(
		gfxAllocContext,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | extraFlags,
		bufferSize,
		data
	);
}
