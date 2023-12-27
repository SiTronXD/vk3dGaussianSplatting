#include "pch.h"
#include "QueueFamilies.h"

QueueFamilies::QueueFamilies()
	: graphicsQueue(VK_NULL_HANDLE),
	presentQueue(VK_NULL_HANDLE)
{
}

QueueFamilies::~QueueFamilies()
{
}

void QueueFamilies::extractQueueHandles(VkDevice device)
{
	// this->indices have been set at this point

	// Get graphics queue handle
	vkGetDeviceQueue(device, this->indices.graphicsFamily.value(), 0, &this->graphicsQueue);

	// Get present queue handle
	vkGetDeviceQueue(device, this->indices.presentFamily.value(), 0, &this->presentQueue);
}

void QueueFamilies::setIndices(const QueueFamilyIndices& indices)
{
	this->indices = indices;
}

QueueFamilyIndices QueueFamilies::findQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	// Get queue family handles
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// Find indices
	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		// Graphics/compute queue family
		if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
		{
			indices.graphicsFamily = i;
		}

		// Present queue family
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			device,
			i,
			surface,
			&presentSupport
		);
		if (presentSupport)
			indices.presentFamily = i;

		// Done
		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}
