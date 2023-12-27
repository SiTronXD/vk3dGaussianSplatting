#pragma once

#include <vector>
#include <glm/glm.hpp>

class TextureData
{
private:

protected:
	uint64_t imageByteSize;

	uint32_t width;
	uint32_t height;

	bool loadTextureUchar(const std::string& filePath, std::vector<unsigned char>& output);
	bool loadTextureFloat(const std::string& filePath, std::vector<float>& output);

	template<typename T>
	void rotateHalfTurn(std::vector<T>& pixels);

public:
	TextureData();
	virtual ~TextureData();

	virtual void copySubtexture(TextureData* srcTextureData, const glm::ivec4& region) = 0;

	virtual bool loadTexture(const std::string& texturePath) = 0;

	inline uint64_t getImageByteSize() const { return this->imageByteSize; }
	inline uint32_t getWidth() const { return this->width; }
	inline uint32_t getHeight() const { return this->height; }
};

template<typename T>
void TextureData::rotateHalfTurn(std::vector<T>& pixels)
{
	for (uint32_t y = 0; y < this->height; ++y)
	{
		for (uint32_t x = 0; x < this->width; ++x)
		{
			uint32_t index = y * this->width + x;
			uint32_t index2 = this->width * this->height - 1 - index;

			index *= 4;
			index2 *= 4;

			// Avoid swapping twice per pixel
			if (index2 <= index)
			{
				x = this->width;
				y = this->height;

				break;
			}

			for (uint32_t c = 0; c < 4; ++c)
			{
				// Swap value
				T tempValue = pixels[index + c];
				pixels[index + c] = pixels[index2 + c];
				pixels[index2 + c] = tempValue;
			}
		}
	}
}