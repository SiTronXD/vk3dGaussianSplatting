// Data per gaussian
struct GaussianData
{
	vec4 position;
	vec4 scale;
};

// Data for sorting per gaussian
struct GaussianSortData
{
	uvec4 num; // uvec4(num, 0, 0, 0)
};