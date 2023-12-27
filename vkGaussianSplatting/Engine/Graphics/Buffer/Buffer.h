#pragma once

#include <vector>
#include <array>

#include <vk_mem_alloc.h>

struct GfxAllocContext;

class Buffer
{
private:
	std::vector<VkBuffer> buffers;
	std::vector<VmaAllocation> bufferMemories;

	const GfxAllocContext* gfxAllocContext;
	VkDeviceSize bufferSize;

	void createBuffer(
		const GfxAllocContext& gfxAllocContext,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VmaAllocationCreateFlags properties,
		VkBuffer& buffer,
		VmaAllocation& bufferMemory);

protected:
	void createStaticGpuBuffer(
		const GfxAllocContext& gfxAllocContext,
		const VkBufferUsageFlags& usageFlags,
		const VkDeviceSize& bufferSize,
		const void* cpuData);

public:
	Buffer();
	virtual ~Buffer();

	void createBuffer(
		const GfxAllocContext& gfxAllocContext,
		const VkDeviceSize& size,
		const VkBufferUsageFlags& usage,
		const VkMemoryPropertyFlags& properties,
		const uint32_t& numBuffers = 1);
	void createStagingBuffer(
		const GfxAllocContext& gfxAllocContext,
		const VkDeviceSize& size);
	void updateBuffer(const void* cpuData);

	static void copyBuffer(
		const GfxAllocContext& gfxAllocContext,
		VkBuffer srcBuffer,
		VkBuffer dstBuffer,
		VkDeviceSize size);

	void cleanup();

	inline const VkBuffer& getVkBuffer(const uint32_t& index = 0) const { return this->buffers[index]; }
	inline const VmaAllocation& getVmaAllocation(const uint32_t& index = 0) const { return this->bufferMemories[index]; }
	inline VkDeviceSize getBufferSize() const { return this->bufferSize; }
};