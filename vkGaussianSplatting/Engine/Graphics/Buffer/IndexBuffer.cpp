#include "pch.h"
#include "IndexBuffer.h"

void IndexBuffer::createIndexBuffer(
	const GfxAllocContext& gfxAllocContext,
	const std::vector<uint32_t>& indices)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	Buffer::createStaticGpuBuffer(
		gfxAllocContext,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		bufferSize,
		indices.data()
	);
}