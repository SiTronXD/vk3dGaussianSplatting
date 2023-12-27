#pragma once

#include "Texture.h"

class Texture2D : public Texture
{
private:
	void copyBufferToImage(
		VkBuffer buffer, VkImage image,
		uint32_t width, uint32_t height);

	bool createTextureImage(const std::string& filePath);

public:
	bool createFromFile(const GfxAllocContext& gfxAllocContext, const std::string& filePath);
	bool createAsDepthTexture(const GfxAllocContext& gfxAllocContext, uint32_t width, uint32_t height, VkImageUsageFlagBits extraUsageFlags = (VkImageUsageFlagBits)0);
	bool createAsDepthSampledTexture(const GfxAllocContext& gfxAllocContext, uint32_t width, uint32_t height);
	bool createAsRenderableTexture(
		const GfxAllocContext& gfxAllocContext,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageUsageFlagBits extraUsageFlags = (VkImageUsageFlagBits)0);
	bool createAsRenderableSampledTexture(
		const GfxAllocContext& gfxAllocContext,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageUsageFlagBits extraUsageFlags = (VkImageUsageFlagBits)0,
		const SamplerSettings& samplerSettings = {});
};