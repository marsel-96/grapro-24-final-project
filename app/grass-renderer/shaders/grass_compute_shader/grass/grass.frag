#version 450 core

#include "include/data.glsl"

in VS_OUT {
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
	vec2 uv;
} fs_in;

uniform vec4 Color;
uniform vec4 BottomColor;
uniform vec4 TopColor;

uniform float ShadingParameter;
uniform float ShadingOffset;

out vec4 FragColor;

uniform vec3 WorldSpaceLightPosition = vec3(0.0, 20.0, 0.0);

void main()
{

	float shading = clamp(
		dot(WorldSpaceLightPosition.xyz, fs_in.normal), 0.0, 1.0
	);

	shading = (shading + ShadingOffset) / ShadingParameter;

	vec4 color = mix(BottomColor, TopColor, fs_in.uv.y);
	FragColor = Color * color;
}