#pragma once

#include <vector>
#include <string>

#include "../Buffer/Buffer.h"

struct GfxAllocContext;

class TextureData;

struct SamplerSettings
{
	VkFilter filter = VK_FILTER_LINEAR;
	VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
};

class Texture
{
private:
	VmaAllocation imageMemory;
	VkSampler sampler;

protected:
	VkImage image;
	VkImageView imageView;
	std::vector<VkImageView> mipImageViews;
	VkFormat format;

	uint32_t width;
	uint32_t height;
	uint32_t mipLevels;

	const GfxAllocContext* gfxAllocContext;

	void createImage(
		uint32_t width,
		uint32_t height,
		uint32_t arrayLayers,
		uint32_t mipLevels,
		VkFormat format,
		VkImageCreateFlags flags,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties);
	void transitionImageLayout(
		VkImage image,
		VkFormat format,
		uint32_t layers,
		VkImageLayout oldLayout,
		VkImageLayout newLayout);

	uint32_t getMaxMipLevels(uint32_t width, uint32_t height);

	bool hasStencilComponent(VkFormat format);
	bool createTextureSampler(const SamplerSettings& samplerSettings = {});

public:
	Texture();
	virtual ~Texture();

	void cleanup();

	// Vulkan
	static VkImageView createImageView(
		VkDevice device,
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspectFlags,
		VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D,
		uint32_t baseArrayLevel = 0,
		uint32_t layerCount = 1,
		uint32_t baseMipLevel = 0,
		uint32_t mipLevelCount = 1
	);
	static VkFormat getDepthBufferFormat();

	inline const VkImage& getVkImage() const { return this->image; }
	inline const VmaAllocation& getVmaAllocation() const { return this->imageMemory; }
	inline const VkImageView& getVkImageView() const { return this->imageView; }
	inline const VkImageView& getVkImageMipView(uint32_t mipLevel) const { return this->mipImageViews[mipLevel]; }
	inline const VkSampler& getVkSampler() const { return this->sampler; }
	inline const VkFormat& getVkFormat() const { return this->format; }
	inline uint32_t getWidth() const { return this->width; }
	inline uint32_t getHeight() const { return this->height; }
	inline uint32_t getMipLevels() const { return this->mipLevels; }
};