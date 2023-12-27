#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class Window;

class VulkanInstance
{
private:
	VkInstance instance;

	uint32_t apiVersion;

public:
	VulkanInstance();
	~VulkanInstance();

	void createInstance(
		bool enableValidationLayers,
		const std::vector<const char*>& instanceExtensions,
		const std::vector<const char*>& validationLayers,
		Window* window);

	void cleanup();

	inline const VkInstance& getVkInstance() const { return this->instance; }
	inline uint32_t getApiVersion() const { return this->apiVersion; }
};

