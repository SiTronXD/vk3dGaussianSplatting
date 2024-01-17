// Data per gaussian
struct GaussianData
{
	vec4 position;
	vec4 scale;
	vec4 color;
};

// Data for sorting per gaussian
struct GaussianSortData
{
	uvec4 data; // uvec4(sortKey, gaussianIndex, 0, 0)
};

// Data modified by culling algorithms
struct GaussianCullData
{
	uvec4 numGaussiansToRender; // uvec4(num, 0, 0, 0)
};