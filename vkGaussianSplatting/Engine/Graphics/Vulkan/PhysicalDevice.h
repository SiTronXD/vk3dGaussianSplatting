#pragma once

#include <vector>
#include "../GpuProperties.h"
#include "QueueFamilies.h"

class Renderer;
class VulkanInstance;
class Surface;

class PhysicalDevice
{
private:
	VkPhysicalDevice physicalDevice;

public:
	PhysicalDevice();
	~PhysicalDevice();

	void pickPhysicalDevice(
		VulkanInstance& instance,
		Surface& surface,
		const std::vector<const char*>& deviceExtensions,
		QueueFamilies& outputQueueFamilies);

	inline const VkPhysicalDevice& getVkPhysicalDevice() const { return this->physicalDevice; }
};

