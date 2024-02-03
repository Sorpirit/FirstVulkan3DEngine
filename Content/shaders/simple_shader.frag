#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in float fragTime;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D texSampler;

void main()
{
	vec3 multColor = vec3(inColor.x * sin(fragTime * 0.5f + 0.3), inColor.y * cos(fragTime * 0.5f + 2), inColor.z + sin(fragTime * 0.5f + 4));
	outColor = vec4(multColor * texture(texSampler, fragTexCoord).rgb, 1.0);
}