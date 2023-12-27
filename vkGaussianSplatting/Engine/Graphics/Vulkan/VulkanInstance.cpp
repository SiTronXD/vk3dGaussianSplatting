#include "pch.h"
#include "VulkanInstance.h"
#include "../GpuProperties.h"
#include "../GfxSettings.h"

VulkanInstance::VulkanInstance()
	: instance(VK_NULL_HANDLE),
	apiVersion(0)
{
}

VulkanInstance::~VulkanInstance()
{
}

void VulkanInstance::createInstance(
	bool enableValidationLayers,
	const std::vector<const char*>& instanceExtensions,
	const std::vector<const char*>& validationLayers,
	Window* window)
{
	if (enableValidationLayers && !GpuProperties::checkValidationLayerSupport(validationLayers))
	{
		Log::error("Validation layers requested are not available.");
	}

	// Application info
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Supreme Void";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Supreme Void";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	// Instance create info
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Get, add and set extensions
	auto extensions = GpuProperties::getRequiredExtensions(*window, enableValidationLayers);
	for (size_t i = 0; i < instanceExtensions.size(); ++i)
		extensions.push_back(instanceExtensions[i]);

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Validation layer debug info for specifically instance create/destroy
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	// Validation features extension for more information
	VkValidationFeaturesEXT validationFeatures{};
	validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	validationFeatures.enabledValidationFeatureCount = static_cast<uint32_t>(GfxSettings::validationFeatureEnables.size());
	validationFeatures.pEnabledValidationFeatures = GfxSettings::validationFeatureEnables.data();
	validationFeatures.disabledValidationFeatureCount = 0;

	// Validation layers
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		// Validation layer debug info for specifically instance create/destroy
		DebugMessenger::populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

		// Print more information
		debugCreateInfo.pNext = &validationFeatures;
	}
	else
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	// Create instance
	if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS)
	{
		Log::error("Failed to create instance.");
	}

	// Save api version for VMA
	this->apiVersion = appInfo.apiVersion;
}

void VulkanInstance::cleanup()
{
	// Destroys both physical device and instance
	vkDestroyInstance(this->instance, nullptr);
}
