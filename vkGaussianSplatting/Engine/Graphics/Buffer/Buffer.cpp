#include "pch.h"
#include "Buffer.h"
#include "../GpuProperties.h"

void Buffer::createBuffer(
	const GfxAllocContext& gfxAllocContext,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VmaAllocationCreateFlags properties,
	VkBuffer& buffer,
	VmaAllocation& bufferMemory)
{
	const VkDevice& device = gfxAllocContext.device->getVkDevice();

	// Create buffer
	VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo vmaCreateInfo{};
	vmaCreateInfo.usage = VMA_MEMORY_USAGE_AUTO; // Recommended by VMA
	vmaCreateInfo.flags = properties;
	if (vmaCreateBuffer(
		*gfxAllocContext.vmaAllocator, 
		&bufferInfo, 
		&vmaCreateInfo, 
		&buffer, 
		&bufferMemory, 
		nullptr) != VK_SUCCESS)
	{
		Log::error("Failed to allocate buffer through VMA.");
	}
}

Buffer::Buffer()
	: gfxAllocContext(nullptr),
	bufferSize(0)
{
}

Buffer::~Buffer()
{
}

void Buffer::createBuffer(
	const GfxAllocContext& gfxAllocContext, 
	const VkDeviceSize& bufferSize,
	const VkBufferUsageFlags& usage, 
	const VkMemoryPropertyFlags& properties,
	const uint32_t& numBuffers)
{
	this->gfxAllocContext = &gfxAllocContext;
	this->bufferSize = bufferSize;

	this->buffers.resize(numBuffers, VK_NULL_HANDLE);
	this->bufferMemories.resize(numBuffers, VK_NULL_HANDLE);
	for (uint32_t i = 0; i < numBuffers; ++i)
	{
		this->createBuffer(
			gfxAllocContext,
			this->bufferSize,
			usage,
			properties,
			this->buffers[i],
			this->bufferMemories[i]
		);
	}
}

void Buffer::createStagingBuffer(
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

void Buffer::createStaticGpuBuffer(
	const GfxAllocContext& gfxAllocContext, 
	const VkBufferUsageFlags& usageFlags,
	const VkDeviceSize& bufferSize, 
	const void* cpuData)
{
	// Create staging buffer
	Buffer stagingBuffer;
	stagingBuffer.createStagingBuffer(
		gfxAllocContext,
		bufferSize
	);

	// Copy data to staging buffer
	stagingBuffer.updateBuffer(cpuData);

	// Create real buffer
	this->createBuffer(
		gfxAllocContext,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags,
		0
	);

	// Copy from staging buffer to real buffer
	Buffer::copyBuffer(
		gfxAllocContext,
		stagingBuffer.getVkBuffer(),
		this->getVkBuffer(),
		bufferSize
	);

	// Deallocate staging buffer
	stagingBuffer.cleanup();
}

void Buffer::updateBuffer(const void* cpuData)
{
	const VmaAllocation& currentBufferMemory =
		this->bufferMemories[GfxState::getFrameIndex() % this->bufferMemories.size()];

	// Map buffer memory into CPU accessible memory
	void* data;
	VkResult result = vmaMapMemory(
		*this->gfxAllocContext->vmaAllocator,
		currentBufferMemory,
		&data
	);
	if (result != VK_SUCCESS)
		Log::error("Failed to map buffer memory.");

	// Copy data to memory
	memcpy(
		data, 
		cpuData, 
		size_t(this->bufferSize)
	);

	// Unmap buffer memory
	vmaUnmapMemory(
		*this->gfxAllocContext->vmaAllocator, 
		currentBufferMemory
	);
}

void Buffer::copyBuffer(
	const GfxAllocContext& gfxAllocContext,
	VkBuffer srcBuffer, 
	VkBuffer dstBuffer, 
	VkDeviceSize size)
{
	// Begin command buffer
	CommandBuffer commandBuffer;
	commandBuffer.beginSingleTimeUse(gfxAllocContext);
	
	// Record copy buffer
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(
		commandBuffer.getVkCommandBuffer(), 
		srcBuffer, 
		dstBuffer, 
		1, 
		&copyRegion
	);

	// End command buffer
	commandBuffer.endSingleTimeUse(gfxAllocContext);
}

void Buffer::cleanup()
{
	for (size_t i = 0; i < this->buffers.size(); ++i)
	{
		vmaDestroyBuffer(
			*this->gfxAllocContext->vmaAllocator, 
			this->buffers[i], 
			this->bufferMemories[i]
		);
	}
	this->buffers.clear();
	this->buffers.shrink_to_fit();
	this->bufferMemories.clear();
	this->bufferMemories.shrink_to_fit();
}
