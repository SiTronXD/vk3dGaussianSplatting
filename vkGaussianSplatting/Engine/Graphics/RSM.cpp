#include "pch.h"
#include "RSM.h"

RSM::RSM()
	: position(1.0f),
	forwardDir(0.0f, 0.0f, 1.0f),
	projectionMatrix(1.0f),
	viewMatrix(1.0f)
{
}

void RSM::init(const GfxAllocContext& gfxAllocContext)
{
	this->positionTexture.createAsRenderableTexture(
		gfxAllocContext, 
		RSM::TEX_SIZE,
		RSM::TEX_SIZE,
		RSM::POSITION_FORMAT,
		VK_IMAGE_USAGE_STORAGE_BIT
	);
	this->normalTexture.createAsRenderableTexture(
		gfxAllocContext,
		RSM::TEX_SIZE,
		RSM::TEX_SIZE,
		RSM::NORMAL_FORMAT,
		VK_IMAGE_USAGE_STORAGE_BIT
	);
	this->brdfIndexTexture.createAsRenderableTexture(
		gfxAllocContext,
		RSM::TEX_SIZE,
		RSM::TEX_SIZE,
		RSM::BRDF_INDEX_FORMAT,
		VK_IMAGE_USAGE_STORAGE_BIT
	);
	this->depthTexture.createAsDepthTexture(
		gfxAllocContext,
		RSM::TEX_SIZE,
		RSM::TEX_SIZE
	);

	this->highResShadowMapTexture.createAsDepthSampledTexture(
		gfxAllocContext,
		RSM::HIGH_RES_SHADOW_MAP_SIZE,
		RSM::HIGH_RES_SHADOW_MAP_SIZE
	);

	// Light cam ubo
	this->lightCamUbo.createDynamicCpuBuffer(
		gfxAllocContext,
		sizeof(LightCamUBO)
	);

	this->projectionMatrix = glm::perspective(
		glm::radians(90.0f),
		1.0f,
		0.1f,
		100.0f
	);

	// Orientation
	this->setOrientation(
		glm::vec3(-1.0f, 1.0f, 1.0f), 
		glm::vec3(0.0f, 0.0f, 0.0f)
	);
}

void RSM::setOrientation(const glm::vec3& position, const glm::vec3& lookAtPosition)
{
	const glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	this->position = position;
	this->forwardDir = glm::normalize(lookAtPosition - this->position);
	this->viewMatrix = glm::lookAt(
		this->position,
		this->position + this->forwardDir,
		std::abs(glm::dot(this->forwardDir, worldUp)) < 1.0f ? worldUp : glm::vec3(0.0f, 0.0f, 1.0f)
	);
}

void RSM::update()
{
	// Update cam buffer
	LightCamUBO ubo{};
	ubo.vp = this->projectionMatrix * this->viewMatrix;
	ubo.pos = glm::vec4(this->position, (float) RSM::TEX_SIZE);
	ubo.dir = glm::vec4(this->forwardDir, 0.0f);
	this->lightCamUbo.updateBuffer(&ubo);
}

void RSM::cleanup()
{
	this->lightCamUbo.cleanup();

	this->highResShadowMapTexture.cleanup();

	this->depthTexture.cleanup();
	this->brdfIndexTexture.cleanup();
	this->normalTexture.cleanup();
	this->positionTexture.cleanup();
}
