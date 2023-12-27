#pragma once

#include "Vulkan/Device.h"
#include "Vulkan/CommandPool.h"

struct GfxAllocContext
{
	Device* device = nullptr;
	QueueFamilies* queueFamilies = nullptr;
	CommandPool* singleTimeCommandPool = nullptr;
	VmaAllocator* vmaAllocator = nullptr;
};