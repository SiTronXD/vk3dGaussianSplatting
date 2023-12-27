#pragma once

#include "../SMath.h"

struct CamUBO
{
	glm::mat4 vp;
	glm::vec4 pos;
};

struct LightCamUBO
{
	glm::mat4 vp;
	glm::vec4 pos;
	glm::vec4 dir;
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
	glm::uvec4 resolution; // uvec4(width, height, 0, 0)
};

struct PrefilterPCD
{
	glm::uvec4 resolution; // uvec4(mip 0 width, mip 0 height, mip N width, mip N height)
	glm::vec4 roughness; // vec4(roughness, 0.0f, 0.0f, 0.0f)
};

#define MAX_L 6
#define NUM_SH_COEFFICIENTS ((MAX_L + 1) * (MAX_L + 1))
#define NUM_SHADER_SH_COEFFS (NUM_SH_COEFFICIENTS * 3)
struct SHData
{
	// Ceil NUM_SHADER_SH_COEFFS to be divisible by 4, for 16-byte alignment
	float coefficients[((NUM_SHADER_SH_COEFFS + 3) / 4) * 4];
};