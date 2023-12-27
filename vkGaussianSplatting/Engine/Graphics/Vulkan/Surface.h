#pragma once

#include "VulkanInstance.h"

class Window;

class Surface
{
private:
	VkSurfaceKHR surface;

	const VulkanInstance* instance;

public:
	Surface();
	~Surface();

	void createSurface(
		const VulkanInstance& instance,
		Window& window);
	void cleanup();

	inline const VkSurfaceKHR& getVkSurface() const { return this->surface; }
};

