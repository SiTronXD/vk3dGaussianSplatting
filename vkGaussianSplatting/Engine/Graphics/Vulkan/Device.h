#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "QueueFamilies.h"

class PhysicalDevice;

class Device
{
private:
	VkDevice device;

public:
	Device();
	~Device();

	void createDevice(
		PhysicalDevice& physicalDevice,
		const std::vector<const char*>& deviceExtensions,
		const std::vector<const char*>& validationLayers,
		bool enableValidationLayers,
		const QueueFamilyIndices& queueFamilyIndices);

	void waitIdle() const;
	void cleanup();

	inline const VkDevice& getVkDevice() const { return this->device; }
};