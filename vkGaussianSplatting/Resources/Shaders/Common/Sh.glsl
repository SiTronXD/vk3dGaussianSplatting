#extension GL_GOOGLE_include_directive: require

#include "ShEfficientEval.glsl"
#include "AssocLegEfficientEval.glsl"

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define SQRT_PI 1.7724538509055160272981674833411
#define SQRT_TWO 1.4142135623730950488016887242097

#define RSM_FOV (HALF_PI)
#define COS_HALF_FOV 0.70710678118654752440084436210485
#define PRIMARY_LIGHT_POWER 400.0f
#define PRIMARY_LIGHT_POWER_HVL (PRIMARY_LIGHT_POWER) // This can be used separately to enhance indirect light
#define MAX_RSM_SIZE 8

#define NUM_ANGLES 90
#define MAX_L 6
#define DIRECT_CONVOLUTION_L 4
#define CONVOLUTION_L 4
#define HVL_EMISSION_L 2
#define NUM_SH_COEFFICIENTS ((MAX_L + 1) * (MAX_L + 1))
#define NUM_SHADER_SH_COEFFS (NUM_SH_COEFFICIENTS * 3)
struct SHData
{
	float coeffs[((NUM_SHADER_SH_COEFFS + 3) / 4) * 4];
};
layout(binding = 5) readonly buffer SHCoefficientsBuffer
{
	SHData coefficientSets[];
} shCoefficients;

layout(binding = 6) uniform sampler2D shadowMapTex;
layout(binding = 7, rgba32f) uniform readonly image2D rsmPositionTex;
layout(binding = 8, rgba32f) uniform readonly image2D rsmNormalTex;
layout(binding = 9, r8ui) uniform readonly uimage2D rsmBRDFIndexTex;

// 1024 * 16 = 16384 bytes are guaranteed to be available in Vulkan
shared vec3 hvlEmissionCoefficients[MAX_RSM_SIZE * MAX_RSM_SIZE * ((HVL_EMISSION_L + 1) * (HVL_EMISSION_L + 1))];

int factorial(int v)
{
    int t = 1;
    for (int i = 2; i <= v; ++i)
    {
        t *= i;
    }
    return t;
}

// Associated Legendre Polynomial P(l,m,x) at x
// For more, see “Numerical Methods in C: The Art of Scientific Computing”, Cambridge University Press, 1992, pp 252-254" 
// Further explanation: https://www.cse.chalmers.se/~uffe/xjobb/Readings/GlobalIllumination/Spherical%20Harmonic%20Lighting%20-%20the%20gritty%20details.pdf
float P(int l, int m, float x)
{
    float pmm = 1.0;

    if (m > 0)
    {
        float somx2 = sqrt((1.0 - x) * (1.0 + x));
        float fact = 1.0;
        for (int i = 1; i <= m; ++i)
        {
            pmm *= -fact * somx2;
            fact += 2.0;
        }
    }

    if (l == m)
        return pmm;

    float pmmp1 = x * (2.0 * float(m) + 1.0) * pmm;
    if (l == m + 1)
        return pmmp1;

    float pll = 0.0;
    for (int ll = m + 2; ll <= l; ++ll)
    {
        pll = ((2.0 * float(ll) - 1.0) * x * pmmp1 - float(ll + m - 1) * pmm) / float(ll - m);
        pmm = pmmp1;
        pmmp1 = pll;
    }

    return pll;
}

// Normalization factor
float K(int l, int m)
{
    float numer = float((2 * l + 1) * factorial(l - abs(m)));
    float denom = 4.0 * PI * float(factorial(l + abs(m)));
    return sqrt(numer / denom);
}

// SH basis function.
// Explanation: https://www.ppsloan.org/publications/StupidSH36.pdf
float y(int l, int m, float cosTheta, float phi)
{
    // Zonal harmonics as base case where m = 0
    float v = K(l, m);
    v *= P(l, abs(m), cosTheta);

    // Remaining cases
    if (m != 0)
        v *= sqrt(2.0);
    if (m > 0)
        v *= cos(float(m) * phi);
    if (m < 0)
        v *= sin(float(abs(m)) * phi);

    return v;
}

mat3x3 getWorldToTangentMat(vec3 normal, vec3 outgoingDir)
{
    // Based on coordinate system provided in "A Data-Driven Reflectance Model"
    // by Wojciech Matusik, Hanspeter Pfister, Matt Brand, Leonard McMillan
    // http://www.csbio.unc.edu/mcmillan/pubs/sig03_matusik.pdf

    // Handle case where normal == outgoingDir
    if(dot(normal, outgoingDir) >= 0.9999f) outgoingDir = vec3(0.0f, 1.0f, 0.0f);
    if(dot(normal, outgoingDir) >= 0.9999f) outgoingDir = vec3(1.0f, 0.0f, 0.0f);

    // Tangent vectors
    vec3 tangentLeft = normalize(cross(normal, outgoingDir));
    vec3 tangentForward = cross(tangentLeft, normal);

    return transpose(mat3x3(tangentLeft, normal, tangentForward));
}

uint getBrdfVectorIndex(vec3 normal, vec3 viewDir, uint brdfIndex)
{
    const float cosAngle = dot(normal, viewDir);
    const float angle = acos(clamp(cosAngle, 0.0f, 1.0f));
    const int angleIndex = clamp(
        int(angle / HALF_PI * float(NUM_ANGLES)), 
        0, 
        NUM_ANGLES - 1
    );

    return (brdfIndex * NUM_ANGLES * 2) + (angleIndex);
}

uint getBrdfCosVectorIndex(vec3 normal, vec3 viewDir, uint brdfIndex)
{
    return getBrdfVectorIndex(normal, viewDir, brdfIndex) + NUM_ANGLES;
}

void insertHvlCoeffsIntoSharedMem(ivec2 hvlIndex, vec3 lightPos)
{
    vec3 hvlNormal = imageLoad(rsmNormalTex, hvlIndex).rgb;
    vec3 hvlPos = imageLoad(rsmPositionTex, hvlIndex).rgb;
    vec3 hvlToPrimaryLight = normalize(lightPos - hvlPos);
    uint yBrdfIndex = imageLoad(rsmBRDFIndexTex, hvlIndex).r;

    const SHData FPrime = 
        shCoefficients.coefficientSets[getBrdfVectorIndex(hvlNormal, hvlToPrimaryLight, yBrdfIndex)];
    
    // Loop through coefficients
    const int HVL_EMISSION_NUM_COEFFS = (HVL_EMISSION_L + 1) * (HVL_EMISSION_L + 1);
    for(int i = 0; i < HVL_EMISSION_NUM_COEFFS; ++i)
    {
        hvlEmissionCoefficients[
            (hvlIndex.y * MAX_RSM_SIZE + hvlIndex.x) * HVL_EMISSION_NUM_COEFFS 
            + i
        ] = vec3(
                FPrime.coeffs[i * 3 + 0],  // R
                FPrime.coeffs[i * 3 + 1],  // G
                FPrime.coeffs[i * 3 + 2]   // B
            );
    }
}

// Proportion of HVL that lies within shading hemisphere
float getH(float halfAngle, vec3 xNormal, vec3 wj)
{
    float aHighBar = halfAngle + HALF_PI;
    float aLowBar = halfAngle - HALF_PI;
    float angle = acos(clamp(dot(xNormal, wj), -1.0f, 1.0f));
    angle = clamp(angle, aLowBar, aHighBar);

    float x = (aHighBar - angle) / (2.0 * halfAngle);
    x = clamp(x, 0.0f, 1.0f);
    float s = (3.0 * x * x) - (2.0 * x * x * x); // Smoothstep

    return s;
}

// Geometric factor, corrects energy of spherical shape
float getG(float halfAngle, float radius, vec3 jNormal, vec3 xNormal, vec3 mWj)
{
    float factor = 1.0 / (PI * radius * radius);
    float dotNjMWj = max(dot(jNormal, mWj), 0.0f);
    float h = getH(halfAngle, xNormal, -mWj);

    return factor * dotNjMWj * h;
}

// Incoming luminance from HVL to shaded point
vec3 getLj(float fRsmSize, float halfAngle, float radius, vec3 jNormal, vec3 xNormal, vec3 mWj, vec3 hvlToPrimaryLight, uint yBrdfIndex, ivec2 hvlSampleIndex)
{
    float capitalPhi = PRIMARY_LIGHT_POWER_HVL / (fRsmSize * fRsmSize);
    float g = getG(halfAngle, radius, jNormal, xNormal, mWj);
    
    // Relative light direction
    vec3 wLightTangentSpace = getWorldToTangentMat(jNormal, hvlToPrimaryLight) * mWj;

    #if (HVL_EMISSION_L == 2)
        float shBasisFuncValues[9];
        SHEval3(
            wLightTangentSpace,
            shBasisFuncValues
        );
    #endif

    // Obtain coefficient vector F (without cosine term)
    //const SHData FPrime = shCoefficients.coefficientSets[getBrdfVectorIndex(jNormal, hvlToPrimaryLight, yBrdfIndex)];
    vec3 dotFY = vec3(0.0f);
    const int HVL_EMISSION_NUM_COEFFS = (HVL_EMISSION_L + 1) * (HVL_EMISSION_L + 1);
    for(int i = 0; i < HVL_EMISSION_NUM_COEFFS; ++i)
    {
        dotFY += 
            hvlEmissionCoefficients[
                (hvlSampleIndex.y * MAX_RSM_SIZE + hvlSampleIndex.x) * HVL_EMISSION_NUM_COEFFS 
                + i] * 
            shBasisFuncValues[i];
    }

    return dotFY * capitalPhi * g;
}

float getCoeffLHat(int l, float alpha)
{
	if(l == 0)
	{
		return SQRT_PI * (1.0 - alpha);
	}
	
    #if (CONVOLUTION_L <= 4)
        return sqrt(PI / float(2u * l + 1u)) * (evalAssocLeg(l - 1, alpha) - evalAssocLeg(l + 1, alpha));
    #endif
}

float getFactorCoeffL(int l, float alpha)
{
    float factor = sqrt(4.0 * PI / float(2u * l + 1u));
    float LHat = getCoeffLHat(l, alpha);

    return factor * LHat;
}

vec3 getIndirectLight(
    vec3 worldPos, 
    vec3 lightPos, 
    vec3 normal, 
    vec3 viewDir, 
    int rsmSize, 
    uint xBrdfIndex,
    int startHvlIndex,
    int numHvls)
{
	vec3 color = vec3(0.0f);

    float fRsmSize = float(rsmSize);
    float gamma = SQRT_TWO * RSM_FOV / fRsmSize;
    float taylorTanGamma = (gamma + (gamma * gamma * gamma / 3.0f)); // Taylor series approximation of tan(x)

    // Transform matrix per shaded point
    mat3x3 worldToTangentMat = getWorldToTangentMat(normal, viewDir);

    // Obtain coefficient vector F (with cosine term)
    const SHData F = shCoefficients.coefficientSets[getBrdfCosVectorIndex(normal, viewDir, xBrdfIndex)];
    #if (CONVOLUTION_L == 4)
        float shBasisFuncValues[25];
    #endif

    float factorCoeffL[CONVOLUTION_L + 1];

    // Loop through each HVL
    int x = (startHvlIndex % rsmSize);
    int y = startHvlIndex / rsmSize;
    
    //for(int y = 0; y < rsmSize; ++y)
    for(int hvlIndex = 0; hvlIndex < numHvls; ++hvlIndex)
    {
        //for(int x = 0; x < rsmSize; ++x)
	    {
            ivec2 uvIndex = ivec2(x, y);
            vec3 hvlNormal = imageLoad(rsmNormalTex, uvIndex).rgb;

            // This texel does not contain a valid HVL
            if(hvlNormal.x > 32.0f)
            {
                continue;
            }

            // HVL cache
            vec3 hvlPos = imageLoad(rsmPositionTex, uvIndex).rgb;
            uint yBrdfIndex = imageLoad(rsmBRDFIndexTex, uvIndex).r;

            // HVL data
            vec3 wLight = hvlPos - worldPos;
            float hvlDistance = length(wLight);
            wLight /= hvlDistance;

            vec3 hvlToPrimaryLight = lightPos - hvlPos;
            float d = length(hvlToPrimaryLight);
            hvlToPrimaryLight /= d;
            
            float hvlRadius = d * taylorTanGamma;
            
            // Pythagorean identities
            float sinA = hvlRadius / max(hvlDistance, 0.0001f);
            float alpha = sqrt(clamp(1.0f - sinA*sinA, 0.0f, 1.0f)); // alpha = cos(a)
            float halfAngle = acos(clamp(alpha, 0.0f, 1.0f));

            // Relative light direction
            vec3 wLightTangentSpace = worldToTangentMat * wLight;

            #if (CONVOLUTION_L == 4)
                SHEval5(
                    wLightTangentSpace,
                    shBasisFuncValues
                );
            #endif

            // Precalculate factors for L coefficient
            vec3 dotLF = vec3(0.0f);
            for(int lSH = 0; lSH <= CONVOLUTION_L; ++lSH)
            {
                factorCoeffL[lSH] = getFactorCoeffL(lSH, alpha);
            }
            
            // L . F
            int currL = 0;
            const int CONVOLUTION_NUM_COEFFS = (CONVOLUTION_L + 1) * (CONVOLUTION_L + 1);
            for(int i = 0; i < CONVOLUTION_NUM_COEFFS; ++i)
            {
                // Next l
                if(i >= (currL + 1) * (currL + 1))
                {
                    currL++;
                }

                // Dot
                float Llm = factorCoeffL[currL] * shBasisFuncValues[i];
                dotLF += 
                    Llm * vec3(
                        F.coeffs[i * 3 + 0],    // R
                        F.coeffs[i * 3 + 1],    // G
                        F.coeffs[i * 3 + 2]     // B
                    );
            }

            // Add "Lj(L . F)" from each HVL
            vec3 Lj = getLj(fRsmSize, halfAngle, hvlRadius, hvlNormal, normal, -wLight, hvlToPrimaryLight, yBrdfIndex, uvIndex);
            color += Lj * dotLF;

            // Visualize HVL sizes
            //color += hvlDistance <= hvlRadius ? vec3(0.1f, 0.0f, 0.0f) : vec3(0.0f);

            x++;
            if(x >= rsmSize)
            {
                x = 0;
                y++;
            }
	    }
    }

	return color;
}

vec3 getDirectLight(vec3 worldPos, vec3 lightPos, vec3 normal, vec3 viewDir, uint xBrdfIndex)
{
    vec3 color = vec3(0.0f);

    vec3 toLightVec = lightPos - worldPos;
    vec3 toLight = normalize(toLightVec);

    // Relative light direction
    vec3 wLightTangentSpace = getWorldToTangentMat(normal, viewDir) * toLight;

    #if (DIRECT_CONVOLUTION_L == 4)
        float shBasisFuncValues[25];
        SHEval5(
            wLightTangentSpace,
            shBasisFuncValues
        );
    #endif

    // Obtain coefficient vector F' (without cosine term)
    const SHData FPrime = shCoefficients.coefficientSets[getBrdfVectorIndex(normal, viewDir, xBrdfIndex)];
    const int DIRECT_CONVOLUTION_NUM_COEFFS = (DIRECT_CONVOLUTION_L + 1) * (DIRECT_CONVOLUTION_L + 1);
    for(int i = 0; i < DIRECT_CONVOLUTION_NUM_COEFFS; ++i)
    {
        color += 
            vec3(
                FPrime.coeffs[i * 3 + 0],  // R
                FPrime.coeffs[i * 3 + 1],  // G
                FPrime.coeffs[i * 3 + 2]   // B
            ) * shBasisFuncValues[i];
    }

    // Cos(theta)
    color *= max(dot(normal, toLight), 0.0f);

    color *= PRIMARY_LIGHT_POWER;

	// Spotlight attenuation
	color *= 1.0f / max(dot(toLightVec, toLightVec), 0.0001f);

    return color;
}