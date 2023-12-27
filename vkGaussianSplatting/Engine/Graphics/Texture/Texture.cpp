#include "pch.h"
#include "Texture.h"
#include "../GpuProperties.h"

bool Texture::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		format == VK_FORMAT_D24_UNORM_S8_UINT;
}

uint32_t Texture::getMaxMipLevels(uint32_t width, uint32_t height)
{
	return uint32_t(std::log2(std::max(width, height))) + 1u;
}

void Texture::createImage(
	uint32_t width, 
	uint32_t height,
	uint32_t arrayLayers,
	uint32_t mipLevels,
	VkFormat format,
	VkImageCreateFlags flags,
	VkImageTiling tiling, 
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties)
{
	this->width = width;
	this->height = height;

	// Create image
	VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Used only by the graphics queue
	imageInfo.flags = flags;

	VmaAllocationCreateInfo vmaCreateInfo{};
	vmaCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	vmaCreateInfo.flags = 0;

	if (vmaCreateImage(
		*this->gfxAllocContext->vmaAllocator,
		&imageInfo,
		&vmaCreateInfo,
		&this->image,
		&this->imageMemory,
		nullptr) != VK_SUCCESS)
	{
		Log::error("Failed to create image through VMA.");
	}
}

VkImageView Texture::createImageView(
	VkDevice device,
	VkImage image, 
	VkFormat format, 
	VkImageAspectFlags aspectFlags,
	VkImageViewType imageViewType,
	uint32_t baseArrayLevel,
	uint32_t layerCount,
	uint32_t baseMipLevel,
	uint32_t mipLevelCount)
{
	// Image view info
	VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.image = image;
	viewInfo.viewType = imageViewType;
	viewInfo.format = format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
	viewInfo.subresourceRange.levelCount = mipLevelCount;
	viewInfo.subresourceRange.baseArrayLayer = baseArrayLevel;
	viewInfo.subresourceRange.layerCount = layerCount;

	// Create image view
	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView))
		Log::error("Failed to create texture image view.");

	return imageView;
}

VkFormat Texture::getDepthBufferFormat()
{
	return GpuProperties::findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

void Texture::transitionImageLayout(
	VkImage image, 
	VkFormat format, 
	uint32_t layers,
	VkImageLayout oldLayout, 
	VkImageLayout newLayout)
{
	CommandBuffer commandBuffer;
	commandBuffer.beginSingleTimeUse(*this->gfxAllocContext);

	VkAccessFlags2 srcAccessMask = 0;
	VkAccessFlags2 dstAccessMask = 0;
	VkPipelineStageFlags2 srcStageMask = 0;
	VkPipelineStageFlags2 dstStageMask = 0;
	VkImageAspectFlags imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

	// Update aspect mask for depth/stencil image
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (this->hasStencilComponent(format))
			imageAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	// Access mask: "how is it accessed?"
	// Src/Dst stage: "when can the transfer take place?"

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		srcAccessMask = 0;
		dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		srcAccessMask = 0;
		dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		Log::error("Unsupported layout transition.");
	}

	// Pipeline barrier 2
	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = imageAspectFlags;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = this->mipLevels;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = layers;
	commandBuffer.memoryBarrier(
		srcAccessMask,
		dstAccessMask,
		srcStageMask,
		dstStageMask,
		oldLayout,
		newLayout,
		image,
		subresourceRange
	);

	// End
	commandBuffer.endSingleTimeUse(*this->gfxAllocContext);
}

bool Texture::createTextureSampler(const SamplerSettings& samplerSettings)
{
	// Sampler create info
	VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samplerInfo.magFilter = samplerSettings.filter;
	samplerInfo.minFilter = samplerSettings.filter;
	samplerInfo.addressModeU = samplerSettings.addressMode;
	samplerInfo.addressModeV = samplerSettings.addressMode;
	samplerInfo.addressModeW = samplerSettings.addressMode;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = GpuProperties::getMaxAnisotropy();
	samplerInfo.borderColor = samplerSettings.borderColor;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = this->mipLevels;

	// Create sampler
	if (vkCreateSampler(
		this->gfxAllocContext->device->getVkDevice(),
		&samplerInfo, 
		nullptr, 
		&this->sampler) != VK_SUCCESS)
	{
		Log::error("Failed to create texture sampler.");
		return false;
	}

	return true;
}

Texture::Texture()
	: image(VK_NULL_HANDLE),
	imageMemory(VK_NULL_HANDLE),
	imageView(VK_NULL_HANDLE),
	sampler(VK_NULL_HANDLE),
	format(VK_FORMAT_R8G8B8A8_UNORM),
	gfxAllocContext(nullptr),
	width(0),
	height(0),
	mipLevels(1)
{
}

Texture::~Texture()
{
}

void Texture::cleanup()
{
	const VkDevice& device = this->gfxAllocContext->device->getVkDevice();

	// Mip views
	for (size_t i = 0; i < this->mipImageViews.size(); ++i)
		vkDestroyImageView(device, this->mipImageViews[i], nullptr);
	this->mipImageViews.clear();

	vkDestroySampler(device, this->sampler, nullptr);
	vkDestroyImageView(device, this->imageView, nullptr);
	vmaDestroyImage(*this->gfxAllocContext->vmaAllocator, this->image, this->imageMemory);
}