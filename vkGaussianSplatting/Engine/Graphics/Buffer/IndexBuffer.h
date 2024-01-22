#pragma once

#include "Buffer.h"

class IndexBuffer : public Buffer
{
private:
public:
	void createIndexBuffer(
		const GfxAllocContext& gfxAllocContext,
		const std::vector<uint32_t>& indices);

	inline const VkBuffer& getVkBuffer() const { return Buffer::getVkBuffer(0); }
	inline const VmaAllocation& getVmaAllocation() const { return Buffer::getVmaAllocation(0); }
};