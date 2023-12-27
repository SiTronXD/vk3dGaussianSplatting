#include "pch.h"
#include "FenceArray.h"
#include "Device.h"

FenceArray::FenceArray()
	: device(nullptr)
{
}

FenceArray::~FenceArray()
{
}

bool FenceArray::create(
	Device& device, 
	const uint32_t& numFences, 
	const VkFenceCreateFlags& flags)
{
	this->device = &device;

	// Fence info
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = flags;

	this->fences.resize(numFences);
	for (size_t i = 0; i < numFences; ++i)
	{
		if (vkCreateFence(
			device.getVkDevice(),
			&fenceInfo,
			nullptr,
			&this->fences[i]
		) != VK_SUCCESS)
		{
			Log::error("Could not create fence (index: " + std::to_string(i) + ")");
			return false;
		}
	}

	return true;
}

void FenceArray::reset(const uint32_t& index)
{
	vkResetFences(
		this->device->getVkDevice(), 
		1, 
		&this->fences[index]
	);
}

void FenceArray::cleanup()
{
	for (size_t i = 0; i < this->fences.size(); ++i)
	{
		vkDestroyFence(
			this->device->getVkDevice(), 
			this->fences[i], 
			nullptr
		);
	}
	this->fences.clear();
}
