#version 450 core

#include "include/data.glsl"
#include "shaders/renderer/lighting/lighting.glsl"
#include "shaders/renderer/lighting/blinn-phong.glsl"

in VS_OUT {
	vec3 position;
	vec3 normal;
	vec2 uv;
} fs_in;

out vec4 FragColor;

uniform vec3 Color;
uniform vec3 BottomColor;
uniform vec3 TopColor;

uniform float AmbientReflectance;
uniform float DiffuseReflectance;
uniform float SpecularReflectance;
uniform float SpecularExponent;

uniform vec3 CameraPosition;

float easeOutQuad(float x) {
	return 1 - (1 - x) * (1 - x);
}

void main()
{
	vec3 albedoColor = mix(BottomColor, TopColor, easeOutQuad(fs_in.uv.y)).rgb;

	SurfaceData data;

	data.normal = normalize(gl_FrontFacing? fs_in.normal : -fs_in.normal);
	data.reflectionColor = Color * albedoColor;
	data.ambientReflectance = AmbientReflectance;
	data.diffuseReflectance = DiffuseReflectance;
	data.specularReflectance = SpecularReflectance;
	data.specularExponent = SpecularExponent;

	vec3 position = fs_in.position;
	vec3 viewDir = GetDirection(position, CameraPosition);
	vec3 color = ComputeLighting(position, data, viewDir, true);


	FragColor = vec4(color.rgb, 1);
}