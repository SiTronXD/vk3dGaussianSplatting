#include "pch.h"
#include "GpuProperties.h"

VkPhysicalDevice* GpuProperties::physicalDevice = nullptr;
float GpuProperties::maxAnisotropy = 0.0f;
float GpuProperties::timestampPeriod = 0.0f;
uint32_t GpuProperties::memoryTypeCount = 0;
VkMemoryType GpuProperties::memoryTypes[32]{};

void GpuProperties::updateProperties(
	VkPhysicalDevice* physicalDevice)
{
	GpuProperties::physicalDevice = physicalDevice;

	// Get properties
	VkPhysicalDeviceProperties properties{};
	VkPhysicalDeviceMemoryProperties memProperties{};
	vkGetPhysicalDeviceProperties(*physicalDevice, &properties);
	vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &memProperties);

	// Properties
	GpuProperties::maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	GpuProperties::timestampPeriod = properties.limits.timestampPeriod;

	// Memory properties
	GpuProperties::memoryTypeCount = memProperties.memoryTypeCount;
	for (uint32_t i = 0; i < GpuProperties::memoryTypeCount; ++i)
	{
		GpuProperties::memoryTypes[i] = memProperties.memoryTypes[i];
	}
}

bool GpuProperties::isFormatSupported(
	VkFormat format, 
	VkImageTiling tiling, 
	VkFormatFeatureFlags features)
{
	// Get format properties for format
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(
		*GpuProperties::physicalDevice,
		format,
		&props
	);

	// Check for linear tiling
	if (tiling == VK_IMAGE_TILING_LINEAR &&
		(props.linearTilingFeatures & features) == features)
	{
		return true;
	}
	// Check for optimal tiling
	else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
		(props.optimalTilingFeatures & features) == features)
	{
		return true;
	}

	return false;
}

VkFormat GpuProperties::findSupportedFormat(
	const std::vector<VkFormat>& candidates, 
	VkImageTiling tiling, 
	VkFormatFeatureFlags features)
{
	// Loop through each format candidate
	for (VkFormat format : candidates)
	{
		// Get format properties for candidate
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(
			*GpuProperties::physicalDevice,
			format,
			&props
		);

		// Check for linear tiling
		if (tiling == VK_IMAGE_TILING_LINEAR &&
			(props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		// Check for optimal tiling
		else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
			(props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	Log::error("Failed to find supported format.");
	return candidates[0];
}

void GpuProperties::querySwapchainSupport(
	const Surface& surface, 
	SwapchainSupportDetails& output)
{
	GpuProperties::queryPhysicalDeviceSwapchainSupport(
		surface, 
		*GpuProperties::physicalDevice, 
		output
	);
}

void GpuProperties::queryPhysicalDeviceSwapchainSupport(
	const Surface& surface,
	VkPhysicalDevice& physicalDevice,
	SwapchainSupportDetails& output)
{
	const VkSurfaceKHR& vkSurface = surface.getVkSurface();

	// Fill in capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physicalDevice, vkSurface, &output.capabilities
	);

	// Fill in formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		physicalDevice, vkSurface, &formatCount, nullptr
	);
	if (formatCount != 0)
	{
		output.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			physicalDevice, vkSurface, &formatCount, output.formats.data()
		);
	}

	// Fill in presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		physicalDevice,
		vkSurface,
		&presentModeCount,
		nullptr
	);
	if (presentModeCount != 0)
	{
		output.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physicalDevice,
			vkSurface,
			&presentModeCount,
			output.presentModes.data()
		);
	}
}

bool GpuProperties::checkPhysicalDeviceExtensionSupport(
	const std::vector<const char*>& deviceExtensions,
	const VkPhysicalDevice& device)
{
	// Get available extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(
		device, nullptr,
		&extensionCount, nullptr
	);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(
		device, nullptr,
		&extensionCount, availableExtensions.data()
	);

	// Unique required extensions
	std::set<std::string> requiredExtensions(
		deviceExtensions.begin(), deviceExtensions.end()
	);

	// Remove found extensions
	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	// Have all required extensions been found and removed?
	return requiredExtensions.empty();
}

bool GpuProperties::isPhysicalDeviceSuitable(
	const std::vector<const char*>& deviceExtensions,
	VkPhysicalDevice physDevice,
	const Surface& surface,
	QueueFamilyIndices& outputIndices)
{
	outputIndices = QueueFamilies::findQueueFamilies(surface.getVkSurface(), physDevice);

	// Find required extension support
	bool extensionsSupported = 
		GpuProperties::checkPhysicalDeviceExtensionSupport(
			deviceExtensions, 
			physDevice
		);

	// Swapchain with correct support
	bool swapchainAdequate = false;
	if (extensionsSupported)
	{
		SwapchainSupportDetails swapchainSupport{};
		GpuProperties::queryPhysicalDeviceSwapchainSupport(surface, physDevice, swapchainSupport);
		swapchainAdequate = !swapchainSupport.formats.empty() &&
			!swapchainSupport.presentModes.empty();
	}

	// Get device features
	VkPhysicalDeviceFeatures supportedFeatures{};
	vkGetPhysicalDeviceFeatures(physDevice, &supportedFeatures);

	// Get device properties
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physDevice, &properties);

	// Timestamps
	bool supportsTimestamps = true;
#ifdef RECORD_GPU_TIMES
	supportsTimestamps = properties.limits.timestampComputeAndGraphics;
#endif

	bool foundSuitableDevice = outputIndices.isComplete() &&
		extensionsSupported &&
		swapchainAdequate &&
		supportedFeatures.samplerAnisotropy &&
		supportedFeatures.fillModeNonSolid &&
		VK_API_VERSION_MINOR(properties.apiVersion) >= 3 && // Make sure atleast vulkan 1.3 is supported
		supportsTimestamps; 

	return foundSuitableDevice;
}

std::vector<const char*> GpuProperties::getRequiredExtensions(
	Window& window,
	bool enableValidationLayers)
{
	// Get required instance extensions from the window
	uint32_t extensionCount = 0;
	const char** requiredExtensions;
	window.getInstanceExtensions(requiredExtensions, extensionCount);
	std::vector<const char*> extensions(requiredExtensions, requiredExtensions + extensionCount);

	// Add extension to support validation layers
	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

bool GpuProperties::checkValidationLayerSupport(const std::vector<const char*>& layersToSupport)
{
	// Get layer count
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Get available layers
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Compare layer names
	for (const char* layerName : layersToSupport)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		// One layer was not found. Abort
		if (!layerFound)
			return false;
	}

	return true;
}