#include "pch.h"
#include "Swapchain.h"
#include "GpuProperties.h"

void Swapchain::createImageViews()
{
	// Create an image view for each swapchain image
	this->imageViews.resize(this->images.size());
	for (size_t i = 0; i < this->images.size(); ++i)
	{
		this->imageViews[i] = Texture::createImageView(
			this->gfxAllocContext->device->getVkDevice(),
			this->images[i],
			this->imageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
	}
}

void Swapchain::chooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& availableFormats,
	VkSurfaceFormatKHR& output)
{
	// Find specific format/color space combination
	for (const auto& availableFormat : availableFormats)
	{
		if ((availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM || availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM) &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			output = availableFormat;
			return;
		}
	}

	// Use the first format/color space combination
	Log::error("No optimal swapchain format was found. First format was chosen: " + std::to_string((uint32_t) availableFormats[0].format) + " " + std::to_string((uint32_t)availableFormats[0].colorSpace));
	output = availableFormats[0];
}

void Swapchain::chooseSwapPresentMode(
	const std::vector<VkPresentModeKHR>& availablePresentModes,
	VkPresentModeKHR& output)
{
	// Find specific present mode
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			output = availablePresentMode;
			return;
		}
	}

	// Guaranteed to be available
	output = VK_PRESENT_MODE_FIFO_KHR;
	return;
}

void Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
	VkExtent2D& output)
{
	if (capabilities.currentExtent.width != ~uint32_t(0))
	{
		output = capabilities.currentExtent;
		return;
	}
	else
	{
		// Get framebuffer size
		int width, height;
		this->window->getFramebufferSize(width, height);

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		actualExtent.height = std::clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);

		output = actualExtent;
		return;
	}
}

Swapchain::Swapchain()
	: swapchain(VK_NULL_HANDLE),
	imageFormat(VK_FORMAT_A1R5G5B5_UNORM_PACK16),
	extent(VkExtent2D{}),
	minImageCount(0),
	surface(nullptr),
	gfxAllocContext(nullptr),
	window(nullptr),
	queueFamilies(nullptr)
{
}

Swapchain::~Swapchain()
{
}

void Swapchain::createSwapchain(
	const Surface& surface, 
	const GfxAllocContext& gfxAllocContext,
	const Window& window,
	const QueueFamilies& queueFamilies)
{
	this->surface = &surface;
	this->gfxAllocContext = &gfxAllocContext;
	this->window = &window;
	this->queueFamilies = &queueFamilies;

	// Swapchain support
	SwapchainSupportDetails swapchainSupport{};
	GpuProperties::querySwapchainSupport(surface, swapchainSupport);

	// Format, present mode and extent
	VkSurfaceFormatKHR surfaceFormat{};
	this->chooseSwapSurfaceFormat(swapchainSupport.formats, surfaceFormat);
	VkPresentModeKHR presentMode{};
	this->chooseSwapPresentMode(swapchainSupport.presentModes, presentMode);
	VkExtent2D extent{};
	this->chooseSwapExtent(swapchainSupport.capabilities, extent);

	// Image count
	this->minImageCount = swapchainSupport.capabilities.minImageCount;
	uint32_t imageCount = std::max(this->minImageCount, GfxSettings::SWAPCHAIN_IMAGES);
	if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
	{
		imageCount = swapchainSupport.capabilities.maxImageCount;
	}

	// Swap chain create info
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface.getVkSurface();

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

	// How swap chain images are used across multiple queue families
	const QueueFamilyIndices& indices = this->queueFamilies->getIndices();
	uint32_t queueFamilyIndices[] =
	{
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;	// Clip pixels overlapped by other windows
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create swapchain
	if (vkCreateSwapchainKHR(
		this->gfxAllocContext->device->getVkDevice(),
		&createInfo, 
		nullptr, 
		&this->swapchain) != VK_SUCCESS)
	{
		Log::error("Failed to created swapchain.");
		return;
	}

	// We've only specified the minimum number of images, so the implementation
	// could create more.
	vkGetSwapchainImagesKHR(this->gfxAllocContext->device->getVkDevice(), this->swapchain, &imageCount, nullptr);
	this->images.resize(imageCount);
	vkGetSwapchainImagesKHR(this->gfxAllocContext->device->getVkDevice(), this->swapchain, &imageCount, this->images.data());

	// Save format and extent
	this->imageFormat = surfaceFormat.format;
	this->extent = extent;

	// Create image views
	this->createImageViews();
}

void Swapchain::createFramebuffers()
{
	// Deferred outputs
	this->deferredPositionTexture.createAsRenderableTexture(
		*this->gfxAllocContext,
		this->getWidth(),
		this->getHeight(),
		Swapchain::DEFERRED_POSITION_FORMAT,
		VK_IMAGE_USAGE_STORAGE_BIT
	);
	this->deferredNormalTexture.createAsRenderableTexture(
		*this->gfxAllocContext,
		this->getWidth(),
		this->getHeight(),
		Swapchain::DEFERRED_NORMAL_FORMAT,
		VK_IMAGE_USAGE_STORAGE_BIT
	);
	this->deferredBrdfIndexTexture.createAsRenderableTexture(
		*this->gfxAllocContext,
		this->getWidth(),
		this->getHeight(),
		Swapchain::DEFERRED_BRDF_INDEX_FORMAT,
		VK_IMAGE_USAGE_STORAGE_BIT
	);

	// Create HDR/depth textures
	this->hdrTexture.createAsRenderableTexture(
		*this->gfxAllocContext,
		this->getWidth(),
		this->getHeight(),
		Swapchain::HDR_FORMAT,
		VK_IMAGE_USAGE_STORAGE_BIT
	);
	this->depthTexture.createAsDepthTexture(
		*this->gfxAllocContext,
		this->getWidth(),
		this->getHeight()
	);
}

void Swapchain::recreate()
{
	const Device& device = *this->gfxAllocContext->device;

	// Handle minimization when width/height is 0
	int width = 0, height = 0;
	this->window->getFramebufferSize(width, height);
	while (width == 0 || height == 0)
	{
		this->window->getFramebufferSize(width, height);
		this->window->awaitEvents();
	}

	// Wait for the GPU
	device.waitIdle();

	// Cleanup
	this->cleanup();

	// Recreate
	this->createSwapchain(*this->surface, *this->gfxAllocContext, *this->window, *this->queueFamilies);
	this->createFramebuffers();
}

void Swapchain::cleanup()
{
	const VkDevice& device = this->gfxAllocContext->device->getVkDevice();

	this->cleanupFramebuffers();

	for (auto imageView : this->imageViews)
		vkDestroyImageView(device, imageView, nullptr);

	vkDestroySwapchainKHR(device, this->swapchain, nullptr);
}

void Swapchain::cleanupFramebuffers()
{
	this->deferredPositionTexture.cleanup();
	this->deferredNormalTexture.cleanup();
	this->deferredBrdfIndexTexture.cleanup();
	this->hdrTexture.cleanup();
	this->depthTexture.cleanup();
}