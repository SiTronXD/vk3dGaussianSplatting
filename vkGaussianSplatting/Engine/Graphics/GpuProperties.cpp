#include "pch.h"
#include "GpuProperties.h"
#include "Renderer.h"

VkPhysicalDevice* GpuProperties::physicalDevice = nullptr;
float GpuProperties::maxAnisotropy = 0.0f;
float GpuProperties::timestampPeriod = 0.0f;
uint32_t GpuProperties::memoryTypeCount = 0;
VkMemoryType GpuProperties::memoryTypes[32]{};

bool GpuProperties::assertGpu(bool condition, const std::string& warningMessage)
{
	if (!condition)
	{
		Log::warning(warningMessage);
	}

	return condition;
}

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
	bool foundSuitableDevice = true;
	auto assertFound = [&foundSuitableDevice](bool condition, const std::string& warningMessage)
	{
		foundSuitableDevice = foundSuitableDevice && assertGpu(
			condition,
			warningMessage
		);
	};

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

	// Get device properties for newer vulkan features
	VkPhysicalDeviceSubgroupProperties subgroupProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES };
	VkPhysicalDeviceProperties2 properties2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
	properties2.pNext = &subgroupProperties;
	vkGetPhysicalDeviceProperties2(physDevice, &properties2);

	// Timestamps
	bool supportsTimestamps = true;
#ifdef RECORD_GPU_TIMES
	supportsTimestamps = properties.limits.timestampComputeAndGraphics;
#endif

	// ------------------ Check support based on features/properties ------------------

	// Assert subgroup size
	assertFound(
		subgroupProperties.subgroupSize >= Renderer::RS_BIN_COUNT, 
		"GPU has insufficient subgroup size: " + std::to_string(subgroupProperties.subgroupSize)
	);

	// Timestamps
	assertFound(supportsTimestamps, "GPU does not support timestamps");

	// Sampler anisotropy
	assertFound(supportedFeatures.samplerAnisotropy, "GPU does not support sampler anisotropy");

	// Non-solid fill mode
	assertFound(supportedFeatures.fillModeNonSolid, "GPU does not support non-solid fill mode");

	// Extensions
	assertFound(extensionsSupported, "GPU does not support the required extensions");

	// Swapchain
	assertFound(swapchainAdequate, "Swapchain does not support proper formats or present modes");

	// Queue families
	assertFound(outputIndices.isComplete(), "Proper queue families could not be found");

	// Vulkan 1.3
	assertFound(VK_API_VERSION_MINOR(properties.apiVersion) >= 3, "GPU does not support Vulkan 1.3");

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