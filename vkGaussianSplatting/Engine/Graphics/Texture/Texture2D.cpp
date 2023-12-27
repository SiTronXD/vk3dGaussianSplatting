#include "pch.h"
#include "Texture2D.h"

void Texture2D::copyBufferToImage(
	VkBuffer buffer, 
	VkImage image, 
	uint32_t width, 
	uint32_t height)
{
	// Begin command buffer
	CommandBuffer commandBuffer;
	commandBuffer.beginSingleTimeUse(*this->gfxAllocContext);

	// Copy
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };
	vkCmdCopyBufferToImage(
		commandBuffer.getVkCommandBuffer(),
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	// End command buffer
	commandBuffer.endSingleTimeUse(*this->gfxAllocContext);
}

bool Texture2D::createTextureImage(const std::string& filePath)
{
	const VkDevice& device = this->gfxAllocContext->device->getVkDevice();

	// Load texture data
	TextureDataUchar textureData;
	if (!textureData.loadTexture(filePath))
	{
		return false;
	}

	VkDeviceSize imageSize = textureData.getImageByteSize();

	// Staging buffer
	Buffer stagingBuffer;
	stagingBuffer.createStagingBuffer(
		*this->gfxAllocContext,
		imageSize
	);

	// Copy data to staging buffer
	stagingBuffer.updateBuffer(textureData.getPixels().data());

	// Create image
	this->createImage(
		textureData.getWidth(),
		textureData.getHeight(),
		1,
		1,
		this->format,
		0,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	// Transition and copy could be setup inside a single
	// command buffer to increase throughput

	// Transition layout to transfer dst
	this->transitionImageLayout(
		this->image,
		this->format,
		1,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	// Copy from buffer to image
	this->copyBufferToImage(
		stagingBuffer.getVkBuffer(),
		this->image,
		textureData.getWidth(),
		textureData.getHeight()
	);

	// Transition layout to shader read
	this->transitionImageLayout(
		this->image,
		this->format,
		1,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	// Deallocate staging buffer
	stagingBuffer.cleanup();

	return true;
}


bool Texture2D::createFromFile(
	const GfxAllocContext& gfxAllocContext,
	const std::string& filePath)
{
	this->gfxAllocContext = &gfxAllocContext;

	bool hasCreatedTextureImage = this->createTextureImage(filePath);

	this->imageView = Texture::createImageView(
		this->gfxAllocContext->device->getVkDevice(),
		this->image,
		this->format,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	bool hasCreatedTextureSampler = this->createTextureSampler();

	return hasCreatedTextureImage &&
		hasCreatedTextureSampler;
}

bool Texture2D::createAsDepthTexture(
	const GfxAllocContext& gfxAllocContext,
	uint32_t width, uint32_t height,
	VkImageUsageFlagBits extraUsageFlags)
{
	this->gfxAllocContext = &gfxAllocContext;

	// Get depth format
	this->format = this->getDepthBufferFormat();

	// Create depth image
	this->createImage(
		width,
		height,
		1,
		1,
		this->format,
		0,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | extraUsageFlags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	// Create depth image view
	this->imageView = Texture::createImageView(
		gfxAllocContext.device->getVkDevice(),
		this->image,
		this->format,
		VK_IMAGE_ASPECT_DEPTH_BIT
	);

	// Explicitly transition image layout, although it is not needed
	// outside of a render pass
	this->transitionImageLayout(
		this->image,
		this->format,
		1,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	);

	return true;
}

bool Texture2D::createAsDepthSampledTexture(
	const GfxAllocContext& gfxAllocContext, 
	uint32_t width, 
	uint32_t height)
{
	bool createdDepthTexture = this->createAsDepthTexture(
		gfxAllocContext,
		width,
		height,
		VK_IMAGE_USAGE_SAMPLED_BIT
	);

	SamplerSettings samplerSettings{};
	samplerSettings.filter = VK_FILTER_NEAREST;
	samplerSettings.addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerSettings.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	bool createdSampler = this->createTextureSampler(samplerSettings);

	return createdDepthTexture && createdSampler;
}

bool Texture2D::createAsRenderableTexture(
	const GfxAllocContext& gfxAllocContext,
	uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageUsageFlagBits extraUsageFlags)
{
	this->gfxAllocContext = &gfxAllocContext;
	this->format = format;

	// Check format support
	if (!GpuProperties::isFormatSupported(
		format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
	{
		Log::error("Format with index " + std::to_string(format) + " is not supported.");
	}

	// Create color image
	this->createImage(
		width,
		height,
		1,
		1,
		this->format,
		0,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | extraUsageFlags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	// Create image view
	this->imageView = Texture::createImageView(
		gfxAllocContext.device->getVkDevice(),
		this->image,
		this->format,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	return true;
}

bool Texture2D::createAsRenderableSampledTexture(
	const GfxAllocContext& gfxAllocContext,
	uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageUsageFlagBits extraUsageFlags,
	const SamplerSettings& samplerSettings)
{
	// Check format support
	if (!GpuProperties::isFormatSupported(
		format, 
		VK_IMAGE_TILING_OPTIMAL, 
		VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
	{
		Log::error("Format with index " + std::to_string(format) + " is not supported.");
	}

	bool hasCreatedTexture = this->createAsRenderableTexture(
		gfxAllocContext,
		width,
		height,
		format,
		(VkImageUsageFlagBits)(extraUsageFlags | VK_IMAGE_USAGE_SAMPLED_BIT)
	);
	bool hasCreatedTextureSampler = this->createTextureSampler(samplerSettings);

	return hasCreatedTexture && hasCreatedTextureSampler;
}
