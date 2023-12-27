#pragma once

#include <array>
#include <vulkan/vulkan.h>

class VulkanInstance;

class DebugMessenger
{
private:
	VkDebugUtilsMessengerEXT debugMessenger;

	const VulkanInstance* instance;

	static VkResult createDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);
	static void destroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);

	static VKAPI_ATTR VkBool32 VKAPI_CALL validationLayerDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

public:
	DebugMessenger();
	~DebugMessenger();

	void createDebugMessenger(const VulkanInstance& instance, bool enableValidationLayers);

	void cleanup();

	static void populateDebugMessengerCreateInfo(
		VkDebugUtilsMessengerCreateInfoEXT& createInfo);
};