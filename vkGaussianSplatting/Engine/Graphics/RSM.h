#pragma once

#include "Texture/Texture2D.h"
#include "Buffer/UniformBuffer.h"

class RSM
{
private:
	Texture2D positionTexture;
	Texture2D normalTexture;
	Texture2D brdfIndexTexture;
	Texture2D depthTexture;

	Texture2D highResShadowMapTexture;

	UniformBuffer lightCamUbo;

	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;

	glm::vec3 position;
	glm::vec3 forwardDir;

public:
	static const VkFormat POSITION_FORMAT = VK_FORMAT_R32G32B32A32_SFLOAT;
	static const VkFormat NORMAL_FORMAT = VK_FORMAT_R32G32B32A32_SFLOAT;
	static const VkFormat BRDF_INDEX_FORMAT = VK_FORMAT_R8_UINT;
	static const uint32_t TEX_SIZE = 8;
	static const uint32_t HIGH_RES_SHADOW_MAP_SIZE = 256;

	RSM();

	void init(const GfxAllocContext& gfxAllocContext);
	void setOrientation(const glm::vec3& position, const glm::vec3& forwardDir);
	void update();
	void cleanup();

	inline const Texture2D& getPositionTexture() const { return this->positionTexture; }
	inline const Texture2D& getNormalTexture() const { return this->normalTexture; }
	inline const Texture2D& getBrdfIndexTexture() const { return this->brdfIndexTexture; }
	inline const Texture2D& getDepthTexture() const { return this->depthTexture; }
	inline const Texture2D& getHighResShadowMapTexture() const { return this->highResShadowMapTexture; }

	inline const Buffer& getLightCamUbo() const { return this->lightCamUbo; }
};