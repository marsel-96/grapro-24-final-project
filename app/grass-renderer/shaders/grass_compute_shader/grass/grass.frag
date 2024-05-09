#version 430 core

in VS_OUT {
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec2 uv;
} fs_in;

out vec4 FragColor;

uniform vec4 Color;
uniform vec4 BottomColor;
uniform vec4 TopColor;

void main()
{
	vec4 color = mix(BottomColor, TopColor, fs_in.uv.y);
	FragColor = Color * color;
}
