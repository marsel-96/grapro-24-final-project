#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;
in float Height;

out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D AlbedoTexture;
uniform vec2 ColorTextureScale;

void main()
{
	vec4 color = texture(AlbedoTexture, TexCoord * ColorTextureScale);
	FragColor = Color * color;
}
