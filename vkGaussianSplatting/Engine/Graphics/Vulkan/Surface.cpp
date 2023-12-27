#include "pch.h"
#include "Surface.h"

Surface::Surface()
	: surface(VK_NULL_HANDLE),
	instance(nullptr)
{
}

Surface::~Surface()
{
}

void Surface::createSurface(
	const VulkanInstance& instance,
	Window& window)
{
	this->instance = &instance;

	if (glfwCreateWindowSurface(
		instance.getVkInstance(),
		window.getWindowHandle(),
		nullptr,
		&this->surface) != VK_SUCCESS)
	{
		Log::error("Failed to create window surface.");
		return;
	}
}

void Surface::cleanup()
{
	vkDestroySurfaceKHR(this->instance->getVkInstance(), this->surface, nullptr);
}
