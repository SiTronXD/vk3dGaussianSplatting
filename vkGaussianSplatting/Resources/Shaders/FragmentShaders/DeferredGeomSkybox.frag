#version 450

layout(set = 0, binding = 1) uniform samplerCube cubeSampler;

layout(location = 0) in vec3 fragPos;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out uvec4 outBrdfIndex;

void main()
{
	// Sample cube map
	vec3 color = texture(cubeSampler, fragPos, 0.0f).rgb;

	outPosition = vec4(fragPos, 1.0f);
	outNormal = vec4(color, -1.0f); // w < 0 signifies this fragment to not be shaded
	outBrdfIndex = uvec4(0u, 0u, 0u, 0u);
}