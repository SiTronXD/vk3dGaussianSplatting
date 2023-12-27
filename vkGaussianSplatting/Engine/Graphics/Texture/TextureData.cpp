#include "pch.h"
#include "TextureData.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool TextureData::loadTextureUchar(
	const std::string& filePath, 
	std::vector<unsigned char>& output)
{
	// Load texture
	int texWidth, texHeight, texChannels;
	stbi_uc* tempPixels = stbi_load(
		filePath.c_str(),
		&texWidth,
		&texHeight,
		&texChannels,
		STBI_rgb_alpha
	);
	if (!tempPixels)
	{
		Log::error("Failed to load texture image.");
		return false;
	}

	// Set members after successful load
	this->imageByteSize = uint64_t(texWidth * texHeight * 4 * sizeof(unsigned char));
	this->width = uint32_t(texWidth);
	this->height = uint32_t(texHeight);

	// Set pixels
	output.resize(this->width * this->height * 4);
	for (size_t i = 0; i < output.size(); ++i)
	{
		output[i] = tempPixels[i];
	}

	// Free pixel array
	stbi_image_free(tempPixels);

	return true;
}

bool TextureData::loadTextureFloat(
	const std::string& filePath, 
	std::vector<float>& output)
{
	// Load texture
	int texWidth, texHeight, texChannels;
	float* tempPixels = stbi_loadf(
		filePath.c_str(),
		&texWidth,
		&texHeight,
		&texChannels,
		STBI_rgb_alpha
	);
	if (!tempPixels)
	{
		Log::error("Failed to load texture image.");
		return false;
	}

	// Set members after successful load
	this->imageByteSize = uint64_t(texWidth * texHeight * 4 * sizeof(float));
	this->width = uint32_t(texWidth);
	this->height = uint32_t(texHeight);

	// Set pixels
	output.resize(this->width * this->height * 4);
	for (size_t i = 0; i < output.size(); ++i)
	{
		output[i] = tempPixels[i];
	}

	// Free pixel array
	stbi_image_free(tempPixels);

	return true;
}

TextureData::TextureData()
	: imageByteSize(0),
	width(0),
	height(0)
{
}

TextureData::~TextureData()
{
}