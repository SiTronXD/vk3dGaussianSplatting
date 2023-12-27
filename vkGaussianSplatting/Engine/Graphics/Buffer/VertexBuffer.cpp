#include "pch.h"
#include "VertexBuffer.h"

void VertexBuffer::createVertexBuffer(
	const GfxAllocContext& gfxAllocContext, 
	const std::vector<Vertex>& vertices)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	Buffer::createStaticGpuBuffer(
		gfxAllocContext,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		bufferSize,
		vertices.data()
	);
}