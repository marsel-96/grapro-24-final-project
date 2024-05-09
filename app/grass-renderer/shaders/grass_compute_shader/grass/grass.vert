#version 430 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexTangent;
layout (location = 3) in vec2 TexCoord;

layout(std430, binding = 0) readonly buffer TRANSFORM_MATRICES
{
	mat4 TransformMatrices[];
};

out VS_OUT {
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec2 uv;
} ts_out;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

void main()
{
	// TODO Whyyyyyyyyy
	mat4 precomputedTransform = transpose(TransformMatrices[gl_InstanceID]);

	ts_out.position = VertexPosition;
	ts_out.normal = VertexNormal;
	ts_out.tangent = VertexTangent;
	ts_out.uv = TexCoord;

	gl_Position = ViewProjMatrix * precomputedTransform * vec4(VertexPosition, 1.0);
}
