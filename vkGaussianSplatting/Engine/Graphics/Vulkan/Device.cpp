#include "pch.h"
#include "Device.h"
#include "PhysicalDevice.h"

Device::Device()
	: device(VK_NULL_HANDLE)
{
}

Device::~Device()
{
}

void Device::createDevice(
	PhysicalDevice& physicalDevice,
	const std::vector<const char*>& deviceExtensions,
	const std::vector<const char*>& validationLayers,
	bool enableValidationLayers,
	const QueueFamilyIndices& indices)
{
	// Unique queue families to be used
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies =
	{
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	// Create queue create info structs
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// ---------- Device features ----------

	// 1.0 device features
	VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	deviceFeatures.features.fillModeNonSolid = VK_TRUE;
	deviceFeatures.features.samplerAnisotropy = VK_TRUE;
	deviceFeatures.features.shaderFloat64 = VK_TRUE;

	// 1.3 device features
	VkPhysicalDeviceVulkan13Features deviceFeatures13 { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	deviceFeatures13.dynamicRendering = VK_TRUE;
	deviceFeatures13.synchronization2 = VK_TRUE;

	// ---------- Logical device ----------

	// Logical device create info
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Chain features
	createInfo.pNext = &deviceFeatures;
	deviceFeatures.pNext = &deviceFeatures13;

	// Not used in newer versions of vulkan
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	// Create the logical device
	if (vkCreateDevice(
		physicalDevice.getVkPhysicalDevice(),
		&createInfo,
		nullptr,
		&this->device) != VK_SUCCESS)
	{
		Log::error("Failed to create logical device!");
		return;
	}
}

void Device::waitIdle() const
{
	vkDeviceWaitIdle(this->device);
}

void Device::cleanup()
{
	vkDestroyDevice(this->device, nullptr);
}
