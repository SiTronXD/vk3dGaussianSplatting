#pragma once

#include "TextureData.h"

class TextureDataFloat : public TextureData
{
private:
	std::vector<float> pixels;

public:
	void copySubtexture(TextureData* srcTextureData, const glm::ivec4& region) override;
	void rotateHalfTurn();

	virtual bool loadTexture(const std::string& texturePath) override;

	virtual inline const std::vector<float>& getPixels() const { return this->pixels; }
};