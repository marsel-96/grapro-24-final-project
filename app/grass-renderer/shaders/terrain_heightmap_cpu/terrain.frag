#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;
in float Height;
in float Height2;

out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D AlbedoTexture;
uniform vec2 ColorTextureScale;

void main()
{
	float yScale = 64.0f;
	float yShift = 16.0f;

//	vec4 color = texture(AlbedoTexture, TexCoord * ColorTextureScale);
	vec4 color = vec4((Height + yShift) / 64.0f) + 0.1;
	FragColor = vec4(Height);
}
