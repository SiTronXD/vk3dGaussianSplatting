
#define FOV_Y (3.1415f * 0.5f)

// Culling limit in ndc
#define CULLING_NDC_LIMIT 1.3f

// lower: incorrect rotation at screen edges
// higher: stretched gaussian artifacts at screen edges
#define IN_VIEW_LIMIT 0.8f 

// Has to match TILE_SIZE in Renderer.h
#define TILE_SIZE 16

// 2^32 - 1
#define MAX_UINT32 4294967295u

mat3x3 getRotMat(vec4 rot)
{
	const float r = rot.x;
	const float x = rot.y;
	const float y = rot.z;
	const float z = rot.w;

	// Explanation and reference: https://www.songho.ca/opengl/gl_quaternion.html
	return mat3x3(
		1.0f - 2.0f * y * y - 2.0f * z * z,			2.0f * x * y - 2.0f * r * z,			2.0f * x * z + 2.0f * r * y,
		2.0f * x * y + 2.0f * r * z,				1.0f - 2.0f * x * x - 2.0f * z * z,		2.0f * y * z - 2.0f * r * x,
		2.0f * x * z - 2.0f * r * y,				2.0f * y * z + 2.0f * r * x,			1.0f - 2.0f * x * x - 2.0f * y * y
	);
}

vec3 getCovarianceMatrix(
	float width, 
	float height, 
	vec3 gScale,
	vec4 gRot,
	vec4 gPosV, 
	mat4 viewMat)
{
	// Sigma = R * S * S^T * R^T
	mat3x3 rotMat = getRotMat(gRot);
	mat3x3 scaleMat = mat3x3(gScale.x,	0.0f,		0.0f,
							 0.0f,		gScale.y,	0.0f,
							 0.0f,		0.0f,		gScale.z);
	mat3x3 sigma = rotMat * scaleMat * transpose(scaleMat) * transpose(rotMat);

	// SigmaPrime = J * W * Sigma * W^T * J^T
	mat3x3 W = mat3x3(	viewMat[0][0], viewMat[0][1], viewMat[0][2],
						viewMat[1][0], viewMat[1][1], viewMat[1][2],
						viewMat[2][0], viewMat[2][1], viewMat[2][2]);
	
	float tanFovY = tan(FOV_Y * 0.5f);
	float tanFovX = tanFovY * width / height;
	float focalX = width / (2.0f * tanFovX);
	float focalY = height / (2.0f * tanFovY);

	// Limit gaussians within view, 
	// to mitigate artifacts when gaussians are close to screen borders.
	vec2 lim = vec2(tanFovX, tanFovY) * IN_VIEW_LIMIT;
	vec2 tempPos = gPosV.xy / gPosV.z;
	gPosV.x = clamp(tempPos.x, -lim.x, lim.x) * gPosV.z;
	gPosV.y = clamp(tempPos.y, -lim.y, lim.y) * gPosV.z;

	mat3x3 J = mat3x3(				focalX / gPosV.z,								0.0f,						0.0f,
										0.0f,									focalY / gPosV.z,				0.0f,
						-(focalX * gPosV.x) / (gPosV.z * gPosV.z), -(focalY * gPosV.y) / (gPosV.z * gPosV.z),	0.0f);
	mat3x3 sigmaPrime = J * W * sigma * transpose(W) * transpose(J);

	vec3 cov = vec3(sigmaPrime[0][0], sigmaPrime[0][1], sigmaPrime[1][1]);

	return cov;
}

vec4 getScreenSpacePosition(float width, float height, vec4 gPosV, mat4 projMat)
{
	vec4 screenSpacePos = projMat * gPosV;
	screenSpacePos.xyz /= screenSpacePos.w;
	screenSpacePos.y = -screenSpacePos.y;
	screenSpacePos.xy = (screenSpacePos.xy + vec2(1.0f)) * 0.5f;
	screenSpacePos.xy *= vec2(width, height);

	return screenSpacePos;
}