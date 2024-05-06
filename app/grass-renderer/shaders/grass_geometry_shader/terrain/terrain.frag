#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D AlbedoTexture;
uniform vec2 ColorTextureScale;

void main()
{
	float yScale = 64.0f;
	float yShift = 16.0f;

	vec4 color = texture(AlbedoTexture, TexCoord * ColorTextureScale);
	FragColor = Color * color;
}
