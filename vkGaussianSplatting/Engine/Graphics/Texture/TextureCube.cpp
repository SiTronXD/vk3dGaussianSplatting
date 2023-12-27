#include "pch.h"
#include "TextureCube.h"

void TextureCube::copyBufferToCubeMap(
	VkBuffer buffer,
	VkImage image,
	uint32_t width,
	uint32_t height,
	uint32_t cubeMapFaceByteStride)
{
	// Begin command buffer
	CommandBuffer commandBuffer;
	commandBuffer.beginSingleTimeUse(*this->gfxAllocContext);

	// Copy
	std::vector<VkBufferImageCopy> regions(6, VkBufferImageCopy{});
	for (size_t i = 0; i < regions.size(); ++i)
	{
		regions[i].bufferOffset = i * cubeMapFaceByteStride;
		regions[i].bufferRowLength = 0;
		regions[i].bufferImageHeight = 0;
		regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		regions[i].imageSubresource.mipLevel = 0;
		regions[i].imageSubresource.baseArrayLayer = i;
		regions[i].imageSubresource.layerCount = 1;
		regions[i].imageOffset = { 0, 0, 0 };
		regions[i].imageExtent = { width, height, 1 };
	}
	vkCmdCopyBufferToImage(
		commandBuffer.getVkCommandBuffer(),
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		uint32_t(regions.size()),
		regions.data()
	);

	// End command buffer
	commandBuffer.endSingleTimeUse(*this->gfxAllocContext);
}

template<typename T>
bool TextureCube::createTextureCubeMapImage(
	const std::vector<T>& cubeMapData,
	uint64_t imageByteSize,
	uint32_t width,
	uint32_t height)
{
	VkDeviceSize cubeMapSize = imageByteSize * 6;

	// Staging buffer
	Buffer stagingBuffer;
	stagingBuffer.createStagingBuffer(
		*this->gfxAllocContext,
		cubeMapSize
	);

	// Copy data to staging buffer
	stagingBuffer.updateBuffer(cubeMapData.data());

	// Create image
	VkImageUsageFlagBits extraUsageFlags =
		this->mipLevels <= 1 ? (VkImageUsageFlagBits)0 : VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	this->createImage(
		width,
		height,
		6,
		this->mipLevels,
		this->format,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | extraUsageFlags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	// Transition and copy could be setup inside a single
	// command buffer to increase throughput

	// Transition layout to transfer dst
	this->transitionImageLayout(
		this->image,
		this->format,
		6,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	// Copy from buffer to image
	this->copyBufferToCubeMap(
		stagingBuffer.getVkBuffer(),
		this->image,
		width,
		height,
		imageByteSize
	);

	if (this->mipLevels <= 1)
	{
		// Transition layout to shader read, if mip generation is not needed
		this->transitionImageLayout(
			this->image,
			this->format,
			6,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}

	// Deallocate staging buffer
	stagingBuffer.cleanup();

	return true;
}

bool TextureCube::createTextureCubeMap(const std::string& filePath)
{
	this->format = VK_FORMAT_R32G32B32A32_SFLOAT;

	const VkDevice& device = this->gfxAllocContext->device->getVkDevice();

	// Load texture data
	TextureDataFloat originalTexture;
	originalTexture.loadTexture(filePath);
	std::vector<TextureDataFloat> textureData(6);
	uint32_t width = originalTexture.getWidth() / 3;
	uint32_t height = originalTexture.getHeight() / 4;

	// Right/Left
	textureData[0].copySubtexture(&originalTexture, glm::ivec4(width * 2, height, width, height));
	textureData[1].copySubtexture(&originalTexture, glm::ivec4(0, height, width, height));

	// Top/Bottom
	textureData[2].copySubtexture(&originalTexture, glm::ivec4(width, 0, width, height));
	textureData[3].copySubtexture(&originalTexture, glm::ivec4(width, height * 2, width, height));

	// Front/Back
	textureData[4].copySubtexture(&originalTexture, glm::ivec4(width, height, width, height));
	textureData[5].copySubtexture(&originalTexture, glm::ivec4(width, height * 3, width, height));

	// Rotate back
	textureData[5].rotateHalfTurn();

	this->mipLevels = this->getMaxMipLevels(width, height);

	// Merge texture data into one single std::vector
	std::vector<float> mergedCubeMapData;
	for (size_t i = 0; i < textureData.size(); ++i)
		mergedCubeMapData.insert(mergedCubeMapData.end(), textureData[i].getPixels().begin(), textureData[i].getPixels().end());
	return this->createTextureCubeMapImage(
		mergedCubeMapData,
		textureData[0].getImageByteSize(),
		textureData[0].getWidth(),
		textureData[0].getHeight());
}

bool TextureCube::createTextureCubeMap(const std::vector<std::string>& texturePaths)
{
	const VkDevice& device = this->gfxAllocContext->device->getVkDevice();

	// Load texture data
	std::vector<TextureDataUchar> textureData(texturePaths.size());
	uint32_t width = 0;
	uint32_t height = 0;
	for (size_t i = 0; i < texturePaths.size(); ++i)
	{
		if (!textureData[i].loadTexture(texturePaths[i]))
		{
			return false;
		}

		// Make sure all textures are of the same size
		if (i > 0)
		{
			if (textureData[i].getWidth() != width || textureData[i].getHeight() != height)
			{
				return false;
			}
		}
		else
		{
			width = textureData[i].getWidth();
			height = textureData[i].getHeight();
		}
	}

	this->mipLevels = this->getMaxMipLevels(width, height);

	// Merge texture data into one single std::vector
	std::vector<unsigned char> mergedCubeMapData;
	for (size_t i = 0; i < textureData.size(); ++i)
		mergedCubeMapData.insert(mergedCubeMapData.end(), textureData[i].getPixels().begin(), textureData[i].getPixels().end());

	return this->createTextureCubeMapImage(
		mergedCubeMapData,
		textureData[0].getImageByteSize(),
		textureData[0].getWidth(),
		textureData[0].getHeight()
	);
}

bool TextureCube::createCubeMapMipMaps()
{
	CommandBuffer commandBuffer;
	commandBuffer.beginSingleTimeUse(*this->gfxAllocContext);

	int32_t mipWidth = int32_t(this->width);
	int32_t mipHeight = int32_t(this->height);

	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 6;

	// Generate mip N from N-1
	for (uint32_t i = 1; i < this->mipLevels; ++i)
	{
		subresourceRange.baseMipLevel = i - 1;

		// Mip N-1
		commandBuffer.memoryBarrier(
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			this->image,
			subresourceRange
		);

		// Blit
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 6;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { std::max(mipWidth / 2, 1), std::max(mipHeight / 2, 1), 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 6;
		commandBuffer.blit(
			this->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			this->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			blit
		);

		// Mip N-1
		commandBuffer.memoryBarrier(
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			this->image,
			subresourceRange
		);

		mipWidth /= 2;
		mipHeight /= 2;
	}

	// Last mip
	subresourceRange.baseMipLevel = this->mipLevels - 1;
	commandBuffer.memoryBarrier(
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		this->image,
		subresourceRange
	);

	// End
	commandBuffer.endSingleTimeUse(*this->gfxAllocContext);

	return true;
}

TextureCube::TextureCube()
{
}

bool TextureCube::createCubeMapFromFile(
	const GfxAllocContext& gfxAllocContext,
	const std::vector<std::string>& filePaths)
{
	this->gfxAllocContext = &gfxAllocContext;

	bool hasCreatedTextureImage = false;

	if (filePaths.size() > 1)
		hasCreatedTextureImage = this->createTextureCubeMap(filePaths);	// 6 image files
	else
		hasCreatedTextureImage = this->createTextureCubeMap(filePaths[0]); // 1 image file

	this->createCubeMapMipMaps();

	this->imageView = Texture::createImageView(
		this->gfxAllocContext->device->getVkDevice(),
		this->image,
		this->format,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_VIEW_TYPE_CUBE,
		0,
		6,
		0,
		this->mipLevels
	);

	bool hasCreatedTextureSampler = this->createTextureSampler();

	return hasCreatedTextureImage &&
		hasCreatedTextureSampler;
}

bool TextureCube::createAsRenderableSampledCubeMap(
	const GfxAllocContext& gfxAllocContext,
	uint32_t width,
	uint32_t height,
	uint32_t mipLevels,
	VkFormat format,
	VkImageUsageFlagBits extraUsageFlags)
{
	this->gfxAllocContext = &gfxAllocContext;
	this->format = format;
	this->mipLevels = mipLevels;

	if (this->mipLevels > this->getMaxMipLevels(width, height))
	{
		Log::error("Tried to create more mip levels than possible for renderable sampled cube map.");
	}

	// Create cube image
	this->createImage(
		width,
		height,
		6,
		this->mipLevels,
		this->format,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | extraUsageFlags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	// Create image view
	this->imageView = Texture::createImageView(
		this->gfxAllocContext->device->getVkDevice(),
		this->image,
		this->format,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_VIEW_TYPE_CUBE,
		0,
		6,
		0,
		this->mipLevels
	);

	// Create one image view per mip level
	if (this->mipLevels > 1)
	{
		this->mipImageViews.resize(this->mipLevels);
		for (uint32_t i = 0; i < this->mipLevels; ++i)
		{
			this->mipImageViews[i] = Texture::createImageView(
				this->gfxAllocContext->device->getVkDevice(),
				this->image,
				this->format,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_VIEW_TYPE_CUBE,
				0,
				6,
				i,
				1
			);
		}
	}

	// Create sampler
	this->createTextureSampler();

	return true;
}