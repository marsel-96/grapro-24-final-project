#version 430 core

#include "include/blade.glsl"
#include "include/bezier.glsl"
#include "include/random.glsl"
#include "include/transform.glsl"

#include "include/webgl-noise/noise2D.glsl"

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexTangent;
layout (location = 3) in vec3 VertexBitangent;
layout (location = 4) in vec2 TexCoord;


layout(std430, binding = 0) readonly buffer GRASS_BLADES_SSBO
{
	GrassBlade Blades[];
};

uniform float Time;

uniform float WindSpeed;
uniform vec2 WindDirection;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

const float MAX_GRASS_HEIGHT = 0.43068f;

// All must be in world space
out VS_OUT {
	vec3 position;
	vec3 normal;
	vec2 uv;
} vs_out;

struct ControlPoints {
	vec3 p0;
	vec3 p1;
	vec3 p2;
};

ControlPoints GetBezierControlPoints(GrassBlade blade) {
	vec2 up = vec2(0.0, 1.0);
	vec2 forward = vec2(1.0, 0.0);

	float upLength = blade.tilt;
	float forwardLength = sqrt(pow(blade.height, 2) - pow(blade.tilt, 2));

	vec2 p0 = vec2(0.0, 0.0);
	vec2 p2 = p0 + forward * forwardLength + up * upLength;
	vec2 p1 = (p2 - p0) * blade.sideCurve; // Midpoint

	p1 = p1 + normalize(vec2(-p1.y, p1.x)) * blade.bend;

	vec3 facing = vec3(blade.facing.x, 1.0, blade.facing.y);

	return ControlPoints(vec3(0, 0, 0), vec3(p1.x, p1.y, p1.x) * facing, vec3(p2.x, p2.y, p2.x) * facing);
}

vec2 GetWindOffset(vec2 pos, float time){
	float posOnSineWave = cos(WindDirection.x) * pos.x - sin(WindDirection.y) * pos.y;

	float t     = time + posOnSineWave + 4 * snoise(0.1 * pos);
	float windx = 2 * sin(.5 * t);
	float windy = 1 * sin(1. * t);

	return WindSpeed * vec2(windx, windy);
}

void main()
{
	GrassBlade blade = Blades[gl_InstanceID];

	float t = TexCoord.y;

	ControlPoints controlPoints = GetBezierControlPoints(blade);

	// controlPoints.p2.xz += GetWindOffset(controlPoints.p2.xz, Time);

	vec3 bezierPoint = quadraticBezierCurve(controlPoints.p0, controlPoints.p1, controlPoints.p2, t);
	vec3 tangentBezierPoint = quadraticBezierCurveTangent(controlPoints.p0, controlPoints.p1, controlPoints.p2, t);

	// Make sure the width is normalized based on max width of model
	float modelWidth = VertexPosition.z / MAX_GRASS_HEIGHT;

	vec3 vertexBitangent = normalize(vec3(blade.facing.y, 0.0f, -blade.facing.x));
	vec3 vertexTangent = normalize(tangentBezierPoint);
	vec3 vertexNormal = normalize(cross(vertexBitangent, vertexTangent));
	vec3 vertexPosition = bezierPoint + vertexBitangent * modelWidth * blade.width / 2.0;


	// ----------------- Vertex Output -----------------

	vs_out.position = (WorldMatrix * vec4(blade.position + vertexPosition, 1.0)).xyz;
	vs_out.normal = (WorldMatrix * vec4(vertexNormal, 0)).xyz;
	//vs_out.tangent = (WorldMatrix * vec4(vertexTangent, 0)).xyz;
	//vs_out.bitangent = (WorldMatrix * vec4(vertexBitangent, 0)).xyz;
	vs_out.uv = vec2(modelWidth * 0.5 + 0.5, TexCoord.y);

	gl_Position = ViewProjMatrix * vec4(vs_out.position, 1.0);

}