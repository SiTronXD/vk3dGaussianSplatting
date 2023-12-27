#include "pch.h"
#include "TextureDataUchar.h"

void TextureDataUchar::copySubtexture(TextureData* srcTextureData, const glm::ivec4& region)
{
	Log::error("Copying subtexture of unsigned char texture data is not supported.");
}

bool TextureDataUchar::loadTexture(const std::string& texturePath)
{
	return TextureData::loadTextureUchar(texturePath, this->pixels);
}
