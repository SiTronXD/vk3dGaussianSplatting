#pragma once

#include "Texture/Texture2D.h"

class Surface;
class Window;
class QueueFamilies;
class RenderPass;

struct GfxAllocContext;

class Swapchain
{
private:
	VkSwapchainKHR swapchain;
	VkFormat imageFormat;
	VkExtent2D extent;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;

	//Texture2D depthTexture;

	uint32_t minImageCount;

	const Surface* surface;
	const GfxAllocContext* gfxAllocContext;
	const Window* window;
	const QueueFamilies* queueFamilies;

	void createImageViews();

	void chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats,
		VkSurfaceFormatKHR& output);
	void chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>& availablePresentModes,
		VkPresentModeKHR& output);
	void chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
		VkExtent2D& output);

public:
	Swapchain();
	~Swapchain();

	void createSwapchain(
		const Surface& surface, 
		const GfxAllocContext& gfxAllocContext,
		const Window& window,
		const QueueFamilies& queueFamilies);
	void createFramebuffers();

	void recreate();
	void cleanup();
	void cleanupFramebuffers();

	inline const VkSwapchainKHR& getVkSwapchain() { return this->swapchain; }
	inline const VkFormat& getVkFormat() const { return this->imageFormat; }
	inline const VkExtent2D& getVkExtent() const { return this->extent; }
	inline const VkImage& getVkImage(const uint32_t& index) const { return this->images[index]; }
	inline const VkImageView& getVkImageView(const uint32_t& index) { return this->imageViews[index]; }
	inline const uint32_t& getWidth() const { return this->extent.width; }
	inline const uint32_t& getHeight() const { return this->extent.height; }
	inline const uint32_t& getMinImageCount() const { return this->minImageCount; }
	inline const size_t getImageCount() const { return this->images.size(); }

	//inline const Texture& getDepthTexture() const { return this->depthTexture; }
};