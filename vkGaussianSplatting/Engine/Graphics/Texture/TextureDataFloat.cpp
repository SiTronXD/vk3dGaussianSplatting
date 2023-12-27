#include "pch.h"
#include "TextureDataFloat.h"

void TextureDataFloat::copySubtexture(TextureData* srcTextureData, const glm::ivec4& region)
{
	// Only copy float pixels
	TextureDataFloat* srcData = dynamic_cast<TextureDataFloat*>(srcTextureData);
	if (srcData == nullptr)
		return;

	this->width = uint32_t(region.z);
	this->height = uint32_t(region.w);
	uint32_t numPixelComponents = this->width * this->height * 4;
	this->imageByteSize = uint64_t(numPixelComponents * sizeof(float));

	const std::vector<float> srcFloatPixels = srcData->getPixels();
	this->pixels.resize(numPixelComponents);
	for (uint32_t y = 0; y < this->height; ++y)
	{
		for (uint32_t x = 0; x < this->width; ++x)
		{
			uint32_t index = (y * this->width + x) * 4;

			uint32_t srcX = region.x + x;
			uint32_t srcY = region.y + y;
			uint32_t srcIndex = (srcY * srcData->getWidth() + srcX) * 4;

			for (uint32_t c = 0; c < 4; ++c)
				this->pixels[index + c] = srcFloatPixels[srcIndex + c];
		}
	}
}

void TextureDataFloat::rotateHalfTurn()
{
	TextureData::rotateHalfTurn<float>(this->pixels);
}

bool TextureDataFloat::loadTexture(const std::string& texturePath)
{
	return TextureData::loadTextureFloat(texturePath, this->pixels);
}
