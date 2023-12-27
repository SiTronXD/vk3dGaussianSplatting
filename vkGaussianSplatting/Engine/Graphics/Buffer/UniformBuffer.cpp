#include "pch.h"
#include "UniformBuffer.h"

void UniformBuffer::createDynamicCpuBuffer(
	const GfxAllocContext& gfxAllocContext,
	const VkDeviceSize& bufferSize)
{
	this->createBuffer(
		gfxAllocContext,
		bufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
		VMA_ALLOCATION_CREATE_MAPPED_BIT,
		GfxSettings::FRAMES_IN_FLIGHT
	);
}