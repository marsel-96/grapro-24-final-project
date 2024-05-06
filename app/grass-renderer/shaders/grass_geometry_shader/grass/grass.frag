#version 430 core

in vec2 colorBlend;

out vec4 FragColor;

uniform vec4 Color;
uniform vec4 BottomColor;
uniform vec4 TopColor;

void main()
{
	vec4 color = mix(BottomColor, TopColor, colorBlend.y);
	FragColor = Color * color;
}
