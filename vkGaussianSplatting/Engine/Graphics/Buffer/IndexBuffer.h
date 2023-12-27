#pragma once

#include "Buffer.h"

class IndexBuffer : public Buffer
{
private:
public:
	void createIndexBuffer(
		const GfxAllocContext& gfxAllocContext,
		const std::vector<uint32_t>& indices);
};