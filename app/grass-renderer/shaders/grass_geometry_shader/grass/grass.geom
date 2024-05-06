#version 440 core

const float M_PI = 3.1415926535897932384626433832795;
const int BLADE_SEGMENTS = 3;
const int MAX_VERTICES = BLADE_SEGMENTS * 2 + 1;

layout(points) in;
layout(triangle_strip, max_vertices = MAX_VERTICES) out;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

uniform sampler2D WindDistortionMap;
uniform vec4 WindDistortionMapScaleOffset;
uniform vec4 WindFrequency;
uniform float WindStrength;

uniform float Time;

uniform float BendRotation;
uniform float BladeWidth;
uniform float BladeWidthRandom;
uniform float BladeHeight;
uniform float BladeHeightRandom;

uniform float BladeForward = 0.38;
uniform float BladeCurvature = 2;

in VS_OUT {
	vec3 position;
	vec3 normal;
	vec3 tangent;
} gs_in[];

out vec2 colorBlend;

const mat4 mvc = ViewProjMatrix * WorldMatrix;

// Pseudo random number generator between 0 and 1
float rand (vec2 seed) {
	return fract(
		sin(dot(seed.xy, vec2(12.9898,78.233))) * 43758.5453123
	);
}

mat3 rotate(float angle, vec3 axis) {
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;

	return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
				oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
				oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c
	);
}

vec4 TransformToClipCoordinates(vec3 position) {
	return mvc * vec4(position, 1.0);
}

void GenerateGrassVertex(vec3 vertexPosition, float width, float height, float forward, vec2 blend, mat3 transformMatrix)
{
	gl_Position = TransformToClipCoordinates(
		vertexPosition + transformMatrix * vec3(width, height, forward)
	);
	colorBlend = blend;
	EmitVertex();
}

mat3 ComputeWindRotationMatrix()
{
	vec2 uv = gs_in[0].position.xz * WindDistortionMapScaleOffset.xy + WindDistortionMapScaleOffset.zw + WindFrequency.xy * Time;
	vec2 windSample = (texture(WindDistortionMap, uv).rg * 2 - 1) * WindStrength;
	vec3 wind = normalize(vec3(windSample.x, windSample.y, 0));
	return rotate(M_PI * length(windSample), wind);
}

void main()
{

	vec3 pos = gs_in[0].position;

	mat3 facingRotationMatrix = rotate(rand(pos.xz) * 2 * M_PI, vec3(0, 1, 0));
	mat3 bendRotationMatrix = rotate(rand(pos.zx) * BendRotation * M_PI * 0.5, vec3(-1, 0, 0));
	float height = (rand(pos.zx) * 2 - 1) * BladeHeightRandom + BladeHeight;
	float width = (rand(pos.xz) * 2 - 1) * BladeWidthRandom + BladeWidth;
	mat3 windRotation = ComputeWindRotationMatrix();
	float forward = rand(pos.xz) * BladeForward;

	mat3 transformBladeBase = windRotation * facingRotationMatrix * bendRotationMatrix;
	mat3 transformBladeTop = windRotation * facingRotationMatrix * bendRotationMatrix;

	GenerateGrassVertex(pos, width, 0, 0, vec2(0, 0), transformBladeBase);
	GenerateGrassVertex(pos, -width, 0, 0, vec2(1, 0), transformBladeBase);

	for (int i = 1; i < BLADE_SEGMENTS; i++)
	{
		float t = i / float(BLADE_SEGMENTS);

		// Add below the line declaring float t.
		float segmentHeight = height * t;
		float segmentWidth = width * (1 - t);

		// Add inside the loop, below the line declaring segmentWidth.
		float segmentForward = pow(t, BladeCurvature) * forward;

		GenerateGrassVertex(pos, segmentWidth, segmentHeight, segmentForward, vec2(0, t), transformBladeTop);
		GenerateGrassVertex(pos, -segmentWidth, segmentHeight, segmentForward, vec2(1, t), transformBladeTop);
	}

	// Add just below the loop to insert the vertex at the tip of the blade.
	GenerateGrassVertex(pos, 0, height, forward, vec2(0.5, 1), transformBladeTop);

	EndPrimitive();
}