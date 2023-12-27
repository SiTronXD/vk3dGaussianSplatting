#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
	mat4 vp;
	vec4 pos; // (x, y, z, shadow map size)
	vec4 dir; // (x, y, z, 0.0f)
} ubo;

layout(push_constant) uniform PushConstantData
{
	mat4 modelMat;
	vec4 materialProperties; // vec4(roughness, metallic, 0, 0)
	uvec4 brdfProperties; // uvec4(brdfIndex, 0, 0, 0)
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

void main()
{
	vec4 worldPos = pc.modelMat * vec4(inPosition, 1.0);

	gl_Position = ubo.vp * worldPos;
}