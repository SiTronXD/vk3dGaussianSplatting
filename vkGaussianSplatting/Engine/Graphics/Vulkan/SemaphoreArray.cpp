#include "pch.h"
#include "SemaphoreArray.h"

SemaphoreArray::SemaphoreArray()
	: device(nullptr)
{
}

SemaphoreArray::~SemaphoreArray()
{
}

bool SemaphoreArray::create(
	Device& device,
	const uint32_t& numSemaphores)
{
	this->device = &device;

	// Empty semaphore info
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Create semaphores
	this->semaphores.resize(numSemaphores);
	for (uint32_t i = 0; i < numSemaphores; ++i)
	{
		if (vkCreateSemaphore(
			device.getVkDevice(),
			&semaphoreInfo,
			nullptr,
			&this->semaphores[i]) != VK_SUCCESS)
		{
			Log::error("Could not create semaphore (index: " + std::to_string(i) + ")");
			return false;
		}
	}

	return true;
}

void SemaphoreArray::cleanup()
{
	for (size_t i = 0; i < this->semaphores.size(); ++i)
	{
		vkDestroySemaphore(
			this->device->getVkDevice(),
			this->semaphores[i], 
			nullptr
		);
	}
	this->semaphores.clear();
}
