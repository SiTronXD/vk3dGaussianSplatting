#pragma once

#include "../SMath.h"

// ----------------- Data for push constants -----------------

struct InitSortListPCD
{
	glm::vec4 clipPlanes; // vec4(nearPlane, farPlane, numGaussians, 0)
	glm::uvec4 resolution; // uvec4(width, height, 0, 0)
};

struct SortGaussiansPCD
{
	glm::uvec4 data; // uvec4(algorithm type, h, 0, 0)
};

struct FindRangesPCD
{
	glm::uvec4 data; // uvec4(numTiles, 0, 0, 0)
};

struct RenderGaussiansPCD
{
	glm::uvec4 resolution; // uvec4(width, height, numGaussians, 0)
};


// ----------------- Data for uniform buffers -----------------

struct CamUBO
{
	glm::mat4 viewMat;
	glm::mat4 projMat;
};

// ----------------- Data for storage buffers -----------------

struct GaussianData
{
	glm::vec4 position; // vec4(x, y, z, 0.0f)
	glm::vec4 scale; // vec4(x, y, z, 0.0f)
	glm::vec4 color; // TODO: remove once SH are implemented
};

struct GaussianSortData
{
	glm::uvec4 data; // uvec4(sortKey0, sortKey1, gaussianIndex, 0)
};

struct GaussianCullData
{
	glm::uvec4 numGaussiansToRender; // uvec4(num, 0, 0, 0);
};

struct GaussianTileRangeData
{
	glm::uvec4 range; // uvec4(startIndex, endIndex, 0, 0);
};