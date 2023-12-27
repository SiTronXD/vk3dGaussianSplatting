#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class VulkanInstance;
class Surface;
class PhysicalDevice;
class SupportChecker;
class Window;

struct QueueFamilyIndices;

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class GpuProperties
{
private:
	friend VulkanInstance;
	friend PhysicalDevice;
	friend SupportChecker;

	static VkPhysicalDevice* physicalDevice;

	static float maxAnisotropy;
	static float timestampPeriod;

	static uint32_t memoryTypeCount;
	static VkMemoryType memoryTypes[32];

	static void updateProperties(
		VkPhysicalDevice* physicalDevice);
	static void queryPhysicalDeviceSwapchainSupport(
		const Surface& surface,
		VkPhysicalDevice& physicalDevice,
		SwapchainSupportDetails& output);
	static bool checkPhysicalDeviceExtensionSupport(
		const std::vector<const char*>& deviceExtensions,
		const VkPhysicalDevice& device);
	static bool isPhysicalDeviceSuitable(
		const std::vector<const char*>& deviceExtensions,
		VkPhysicalDevice device,
		const Surface& surface,
		QueueFamilyIndices& outputIndices);

	static std::vector<const char*> getRequiredExtensions(Window& window, bool enableValidationLayers);
	static bool checkValidationLayerSupport(const std::vector<const char*>& layersToSupport);

public:
	static inline float getMaxAnisotropy() { return maxAnisotropy; }
	static inline float getTimestampPeriod() { return timestampPeriod; }

	static const inline uint32_t& getMemoryTypeCount() { return memoryTypeCount; }
	static const inline VkMemoryType& getMemoryType(const uint32_t& index) { return memoryTypes[index]; }

	static bool isFormatSupported(
		VkFormat format,
		VkImageTiling tiling,
		VkFormatFeatureFlags features);
	static VkFormat findSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features);
	static void querySwapchainSupport(
		const Surface& surface,
		SwapchainSupportDetails& output);
};