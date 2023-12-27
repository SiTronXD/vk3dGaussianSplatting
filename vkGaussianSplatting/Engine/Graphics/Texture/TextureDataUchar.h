#pragma once

#include "TextureData.h"

class TextureDataUchar : public TextureData
{
private:
	std::vector<unsigned char> pixels;

public:
	void copySubtexture(TextureData* srcTextureData, const glm::ivec4& region) override;

	virtual bool loadTexture(const std::string& texturePath) override;

	virtual inline const std::vector<unsigned char>& getPixels() const { return this->pixels; }
};