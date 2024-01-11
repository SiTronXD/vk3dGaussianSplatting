#pragma once

#include "../SMath.h"

struct InitSortListPCD
{
	glm::mat4 viewMat;
	glm::vec4 clipPlanes; // vec4(nearPlane, farPlane, numGaussians, 0)
};

struct SortGaussiansPCD
{
	glm::uvec4 data; // uvec4(algorithm type, h, 0, 0)
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
	glm::vec4 color; // TODO: remove once SH are implemented
};

struct GaussianSortData
{
	glm::uvec4 data; // uvec4(sortKey, gaussianIndex, 0, 0)
};