#version 450

layout(binding = 0) uniform UniformBufferObject 
{
	mat4 vp;
	vec4 pos;
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

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out uint fragBrdfIndex;

void main()
{
	vec4 worldPos = pc.modelMat * vec4(inPosition, 1.0f);
	vec3 worldNormal = (pc.modelMat * vec4(inNormal, 0.0f)).xyz;

	gl_Position = ubo.vp * worldPos;
	fragNormal = worldNormal;
	fragWorldPos = worldPos.xyz;
	fragBrdfIndex = pc.brdfProperties.x;
}