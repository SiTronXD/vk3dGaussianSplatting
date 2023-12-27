
#define GAMMA_VALUE 2.2f

vec3 toneMap(vec3 x)
{
	// Reinhard
	//return x / (x + vec3(1.0f));

	// ACES
	const float a = 2.51f;
	const float b = 0.03f;
	const float c = 2.43f;
	const float d = 0.59f;
	const float e = 0.14f;
	return (x * (a * x + b)) / (x * (c * x + d) + e);
}

vec3 srgbToLinear(vec3 col)
{
	return pow(col, vec3(GAMMA_VALUE));
}

vec3 linearToSrgb(vec3 col)
{
	return pow(col, vec3(1.0f / GAMMA_VALUE));
}