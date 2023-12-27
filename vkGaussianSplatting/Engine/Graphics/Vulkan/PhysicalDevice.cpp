#include "pch.h"
#include "PhysicalDevice.h"

PhysicalDevice::PhysicalDevice()
	: physicalDevice(VK_NULL_HANDLE)
{
}

PhysicalDevice::~PhysicalDevice()
{
}

void PhysicalDevice::pickPhysicalDevice(
	VulkanInstance& instance, 
	Surface& surface, 
	const std::vector<const char*>& deviceExtensions,
	QueueFamilies& outputQueueFamilies)
{
	// Get device count
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance.getVkInstance(), &deviceCount, nullptr);

	// No devices found
	if (deviceCount == 0)
	{
		Log::error("Failed to find GPUs with Vulkan support.");
		return;
	}

	// Get device handles
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance.getVkInstance(), &deviceCount, physicalDevices.data());

	// Pick the first best found device
	for (const auto& physDevice : physicalDevices)
	{
		QueueFamilyIndices indices{};

		// Check support
		if (GpuProperties::isPhysicalDeviceSuitable(deviceExtensions, physDevice, surface, indices))
		{
			this->physicalDevice = physDevice;

			// Set indices after finding a suitable device
			outputQueueFamilies.setIndices(indices);

			break;
		}
	}

	// Could not find a proper physical device
	if (this->physicalDevice == VK_NULL_HANDLE)
	{
		Log::error("Failed to find a suitable GPU.");
		return;
	}

	// Proper physical device found
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);

	std::string gpuName = properties.deviceName;
	std::string gpuApiVersion =
		std::to_string(VK_API_VERSION_MAJOR(properties.apiVersion)) + "." +
		std::to_string(VK_API_VERSION_MINOR(properties.apiVersion)) + "." +
		std::to_string(VK_API_VERSION_PATCH(properties.apiVersion));

	Log::write("GPU name: " + gpuName);
	Log::write("GPU supported API version: " + gpuApiVersion + "\n");

	// Update properties for the program
	GpuProperties::updateProperties(&this->physicalDevice);

	// Print limits
	VkPhysicalDeviceLimits deviceLimits = properties.limits;
	/*Log::write("maxPerStageDescriptorSampledImages: " + std::to_string(deviceLimits.maxPerStageDescriptorSampledImages));
	Log::write("maxPerStageDescriptorStorageBuffers: " + std::to_string(deviceLimits.maxPerStageDescriptorStorageBuffers));
	Log::write("maxPerStageDescriptorStorageImages: " + std::to_string(deviceLimits.maxPerStageDescriptorStorageImages));
	Log::write("maxPerStageDescriptorUniformBuffers: " + std::to_string(deviceLimits.maxPerStageDescriptorUniformBuffers));
	Log::write("maxComputeSharedMemorySize: " + std::to_string(deviceLimits.maxComputeSharedMemorySize));*/
}