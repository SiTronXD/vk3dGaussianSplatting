// Data per gaussian
struct GaussianData
{
	// These remain unmodified
	vec4 position;
	vec4 scale;
	vec4 rot;
	vec4 color;
	vec4 shCoeffs[15];

	// This is modified between GPU passes
	vec4 covariance;
};

// Data for sorting per gaussian
struct GaussianSortData
{
	uvec4 data; // uvec4(sortKey0, sortKey1, gaussianIndex, 0)
};

// Data modified by culling algorithms
struct GaussianCullData
{
	uvec4 numGaussiansToRender; // uvec4(num, maxNumSortElements, 0, 0)
};

// Tile ranges
struct GaussianTileRangeData
{
	uvec4 range; // uvec4(startIndex, endIndex, 0, 0);
};