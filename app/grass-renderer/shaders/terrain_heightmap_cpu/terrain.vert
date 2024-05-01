#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec3 WorldPosition;
out vec3 WorldNormal;
out vec2 TexCoord;
out float Height;
out float Height2;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;
uniform sampler2D Heightmap;

const float yScale = 64.0f;
const float yShift = 16.0f;

void main()
{
	Height = texture(Heightmap, VertexTexCoord).r;
	WorldPosition = (WorldMatrix * vec4(VertexPosition.x, Height * yScale - yShift, VertexPosition.z, 1.0)).xyz;
	WorldNormal = (WorldMatrix * vec4(VertexNormal, 0.0)).xyz;
	TexCoord = VertexTexCoord;
	gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);
}
