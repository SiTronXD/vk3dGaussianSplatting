
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

// Efficient SH basis function evaluation for the first 16 coefficients
// Based on "Efficient Spherical Harmonic Evaluation" by Peter-Pike Sloan
// https://www.ppsloan.org/publications/SHJCGT.pdf
void getShEval4(const vec3 evalDir, inout float pSH[16])
{
	// Rotate evaluation direction
	float fX, fY, fZ;
	{
		fX = -evalDir.x;
		fY = -evalDir.y;
		fZ = evalDir.z;
	}

	float fC0,fC1,fS0,fS1,fTmpA,fTmpB,fTmpC;
	float fZ2 = fZ*fZ;

	pSH[0] = 0.2820947917738781f;
	pSH[2] = 0.4886025119029199f*fZ;
	pSH[6] = 0.9461746957575601f*fZ2 + -0.31539156525252f;
	pSH[12] = fZ*(1.865881662950577f*fZ2 + -1.119528997770346f);
	fC0 = fX;
	fS0 = fY;

	fTmpA = -0.48860251190292f;
	pSH[3] = fTmpA*fC0;
	pSH[1] = fTmpA*fS0;
	fTmpB = -1.092548430592079f*fZ;
	pSH[7] = fTmpB*fC0;
	pSH[5] = fTmpB*fS0;
	fTmpC = -2.285228997322329f*fZ2 + 0.4570457994644658f;
	pSH[13] = fTmpC*fC0;
	pSH[11] = fTmpC*fS0;
	fC1 = fX*fC0 - fY*fS0;
	fS1 = fX*fS0 + fY*fC0;

	fTmpA = 0.5462742152960395f;
	pSH[8] = fTmpA*fC1;
	pSH[4] = fTmpA*fS1;
	fTmpB = 1.445305721320277f*fZ;
	pSH[14] = fTmpB*fC1;
	pSH[10] = fTmpB*fS1;
	fC0 = fX*fC1 - fY*fS1;
	fS0 = fX*fS1 + fY*fC1;

	fTmpC = -0.5900435899266435f;
	pSH[15] = fTmpC*fC0;
	pSH[9] = fTmpC*fS0;
}

#define NUM_SH_COEFFS 16
vec3 getShColor(vec3 evalDir, vec4 shCoeffs[NUM_SH_COEFFS], uint sphericalHarmonicsMode)
{
#if NUM_SH_COEFFS == 16

	float shBasisValues[NUM_SH_COEFFS];
	getShEval4(evalDir, shBasisValues);

	vec3 result = vec3(0.0f);
	if (sphericalHarmonicsMode == 0)		// All bands
	{
		for (int i = 0; i < NUM_SH_COEFFS; ++i)
			result += shCoeffs[i].xyz * shBasisValues[i];
	}
	else if (sphericalHarmonicsMode == 1)	// Skip first band
	{
		for (int i = 1; i < NUM_SH_COEFFS; ++i)
			result += shCoeffs[i].xyz * shBasisValues[i];
		result -= vec3(0.5f);
	}
	else if (sphericalHarmonicsMode == 2)	// Only first band
	{
		result += shCoeffs[0].xyz * shBasisValues[0];
	}

	result += vec3(0.5f);
	result = max(result, vec3(0.0f));
	return result;

#endif
}