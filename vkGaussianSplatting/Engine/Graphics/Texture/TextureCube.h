#pragma once

#include "Texture.h"

class TextureCube : public Texture
{
private:
	void copyBufferToCubeMap(
		VkBuffer buffer, VkImage image,
		uint32_t width, uint32_t height,
		uint32_t cubeMapFaceByteStride);

	template<typename T>
	bool createTextureCubeMapImage(
		const std::vector<T>& cubeMapData,
		uint64_t imageByteSize,
		uint32_t width,
		uint32_t height);

	bool createTextureCubeMap(const std::string& filePath);
	bool createTextureCubeMap(const std::vector<std::string>& filePaths);
	bool createCubeMapMipMaps();

public:
	TextureCube();

	bool createCubeMapFromFile(
		const GfxAllocContext& gfxAllocContext, 
		const std::vector<std::string>& filePaths);
	bool createAsRenderableSampledCubeMap(
		const GfxAllocContext& gfxAllocContext,
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		VkFormat format,
		VkImageUsageFlagBits extraUsageFlags = (VkImageUsageFlagBits) 0);
};