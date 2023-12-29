#pragma once

#include "../SMath.h"

struct CamUBO
{
	glm::mat4 vp;
	glm::vec4 pos;
};

// Push constant data (guaranteed 128 bytes)
struct PCD
{
	glm::mat4 modelMat;
	glm::vec4 materialProperties; // vec4(roughness, metallic, 0, 0)
	glm::uvec4 brdfProperties; // uvec4(brdfIndex, 0, 0, 0);
};

struct DeferredLightPCD
{
	glm::uvec4 resolution; // uvec4(width, height, 0, 0)
	glm::vec4 camPos; // vec4(x, y, z, 0.0f)
};

struct PostProcessPCD
{
	//glm::uvec4 resolution; // uvec4(width, height, 0, 0)
	glm::mat4 viewMat;
	glm::mat4 projMat;
};

struct GaussianData
{
	glm::vec4 position; // vec4(x, y, z, 0.0f)
	glm::vec4 scale; // vec4(x, y, z, 0.0f)
};