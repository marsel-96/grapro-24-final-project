#version 430 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexTangent;
layout (location = 3) in vec2 TexCoord;

out VS_OUT {
	vec3 position;
	vec3 normal;
	vec3 tangent;
} ts_out;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

void main()
{
	ts_out.position = VertexPosition;
	ts_out.normal = VertexNormal;
	ts_out.tangent = VertexTangent;

	gl_Position = vec4(ts_out.position, 1.0);
}
