#pragma once

#include <vulkan/vulkan.h>

#include <optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() &&
			presentFamily.has_value();
	}
};

class QueueFamilies
{
private:
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	QueueFamilyIndices indices;

public:
	QueueFamilies();
	~QueueFamilies();

	void extractQueueHandles(
		VkDevice device);
	void setIndices(const QueueFamilyIndices& indices);

	static QueueFamilyIndices findQueueFamilies(
		VkSurfaceKHR surface,
		VkPhysicalDevice device);

	inline VkQueue& getVkGraphicsQueue() { return this->graphicsQueue; }
	inline VkQueue& getVkPresentQueue() { return this->presentQueue; }
	inline const QueueFamilyIndices& getIndices() const { return this->indices; }
};