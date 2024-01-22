#include "pch.h"
#include "StagingBuffer.h"

void StagingBuffer::createStagingBuffer(
	const GfxAllocContext& gfxAllocContext, 
	const VkDeviceSize& size)
{
	this->createBuffer(
		gfxAllocContext,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
		VMA_ALLOCATION_CREATE_MAPPED_BIT |
		VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT // Dedicated memory can be completely freed, and therefore saves cpu RAM after deallocation.
	);
}
