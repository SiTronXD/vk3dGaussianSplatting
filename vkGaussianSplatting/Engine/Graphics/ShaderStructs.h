#pragma once

#include "../SMath.h"

// TODO: remove once gaussan rendering pipeline is finished
struct CamUBO
{
	glm::mat4 vp;
	glm::vec4 pos;
};

// TODO: remove once gaussan rendering pipeline is finished
// Push constant data (guaranteed 128 bytes)
struct PCD
{
	glm::mat4 modelMat;
};

// TODO: remove once gaussan rendering pipeline is finished
struct DeferredLightPCD
{
	glm::uvec4 resolution; // uvec4(width, height, 0, 0)
	glm::vec4 camPos; // vec4(x, y, z, 0.0f)
};



struct RenderGaussiansPCD
{
	glm::uvec4 resolution; // uvec4(width, height, numGaussians, 0)
};

struct RenderGaussiansUBO
{
	glm::mat4 viewMat;
	glm::mat4 projMat;
};

struct GaussianData
{
	glm::vec4 position; // vec4(x, y, z, 0.0f)
	glm::vec4 scale; // vec4(x, y, z, 0.0f)
};