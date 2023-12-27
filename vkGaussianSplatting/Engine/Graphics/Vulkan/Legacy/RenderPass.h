#pragma once

#include <vulkan/vulkan.h>

class Device;

class RenderPass
{
private:
	VkRenderPass renderPass;

	const Device* device;

public:
	RenderPass();
	~RenderPass();

	void createRenderPass(const Device& device, const VkFormat& colorFormat, const VkFormat& depthFormat);
	void createImguiRenderPass(const Device& device, const VkFormat& colorFormat);

	void cleanup();

	inline const VkRenderPass& getVkRenderPass() const { return this->renderPass; }
};