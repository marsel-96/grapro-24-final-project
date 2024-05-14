#version 430 core

#include "include/blade.glsl"
#include "include/bezier.glsl"
#include "include/random.glsl"
#include "include/transform.glsl"

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexTangent;
layout (location = 3) in vec3 VertexBitangent;
layout (location = 4) in vec2 TexCoord;

out VS_OUT {
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
	vec2 uv;
} vs_out;

layout(std430, binding = 0) readonly buffer GRASS_BLADES_SSBO
{
	GrassBlade Blades[];
};

uniform float Offset;
uniform float Height;
uniform vec2 WindDirection;
uniform float Time;
uniform float WindNoiseScale;
uniform float NoiseOffset;
uniform float MeshDeformationLimitTop;
uniform float MeshDeformationLimitLow;
uniform float WindSpeed;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

void main()
{

	vec4 position = vec4(VertexPosition, 1.0);
	vec4 normal = vec4(VertexNormal, 0.0);
	vec4 tangent = vec4(VertexTangent, 0.0);
	vec4 bitangent = vec4(VertexBitangent, 0.0);

	vs_out.uv = TexCoord;

	GrassBlade blade = Blades[gl_InstanceID];

	position *= vec4(0.15, 0.15, 0.15, 1.0);

	vec3 spawnPos = blade.position.xyz;

	// Use Bezier curve to deform the shape
	vec3 up_vector = vec3(0.0, 0.0, 1.0);
	vec3 p0 = vec3(0.0, 0.0, 0.0);
	vec3 p2 = vec3(sqrt(Offset), sqrt(Height), 0.0);

	float project_length = length(p2 - p0 - up_vector * dot(p2 - p0, up_vector));
	vec3 p1 = p0 + Height * up_vector * max(1 - project_length / Height, 0.05 * max(project_length / Height, 1.0));

	vec3 knotPoint = quadraticBezierCurve(p0, p1, p2, vs_out.uv.y);

	// Apply knot point to vertex
	position += vec4(knotPoint, 0.0);
	tangent = normalize(vec4(quadraticBezierCurveTangent(p0, p1, p2, vs_out.uv.y), 0.0f));

	// Rotate along y-axis with a random degree
	float randomAngle = random(spawnPos.xz) * 360.0;
	mat4 rotateMatrix = rotateXY(randomAngle);

	position = rotateMatrix * position;
	normal = rotateMatrix * normal;
	tangent = rotateMatrix * tangent;

	vec4 worldVertPos = vec4(spawnPos, 0.0) + WorldMatrix * position;

	// Use world uv to sample noise from win texture
	vec2 world_uv = worldVertPos.xz + WindDirection * Time;
	float local_noise = 0.0;

	simpleNoise(world_uv, WindNoiseScale, local_noise);

	local_noise += NoiseOffset;

	// Keep bottom part of mesh at its position
	float smoothedDeformation = smoothstep(MeshDeformationLimitLow, MeshDeformationLimitTop, vs_out.uv.y);
	float distortion = smoothedDeformation * local_noise;

	// Apply distortion
	worldVertPos.xz += distortion * WindSpeed * normalize(WindDirection);
	vec4 objectVertPos = WorldMatrix * worldVertPos;

	gl_Position = ViewProjMatrix * objectVertPos;

}