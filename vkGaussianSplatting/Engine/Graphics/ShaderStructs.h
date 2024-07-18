#pragma once

#include "../SMath.h"

// ----------------- Data for push constants -----------------

struct InitSortListPCD
{
	glm::vec4 clipPlanes; // vec4(nearPlane, farPlane, numGaussians, 0)
	glm::uvec4 resolution; // uvec4(width, height, 0, 0)
};

struct SortGaussiansBmsPCD // Bitonic merge sort
{
	glm::uvec4 data; // uvec4(algorithm type, h, 0, 0)
};

struct SortGaussiansRsPCD // Radix sort
{
	glm::uvec4 data; // uvec4(shiftBits, 0, 0, 0)
};

struct FindRangesPCD
{
	glm::uvec4 data; // uvec4(numSortElements, 0, 0, 0)
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

struct RadixIndirectDispatch // Radix sort
{
	uint32_t countSizeX = 1;
	uint32_t countSizeY = 1;
	uint32_t countSizeZ = 1;

	uint32_t numSortElements;

	uint32_t reduceSizeX = 1;
	uint32_t reduceSizeY = 1;
	uint32_t reduceSizeZ = 1;
	uint32_t padding;
};

struct GaussianData
{
	// These remain unmodified
	glm::vec4 position; // vec4(x, y, z, 0.0f)
	glm::vec4 scale; // vec4(x, y, z, 0.0f)
	glm::vec4 rot = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 color; // vec4(r, g, b, alpha)
	glm::vec4 shCoeffs[15];

	// This is modified between GPU passes
	glm::vec4 covariance; // vec4(cov.x, cov.y, cov.z, 0.0f)
};

struct GaussianSortData
{
	glm::uvec4 data; // uvec4(sortKey0, sortKey1, gaussianIndex, 0)
};

struct GaussianCullData
{
	glm::uvec4 numGaussiansToRender; // uvec4(num, maxNumSortElements, 0, 0);
};

struct GaussianTileRangeData
{
	glm::uvec4 range; // uvec4(startIndex, endIndex, 0, 0);
};