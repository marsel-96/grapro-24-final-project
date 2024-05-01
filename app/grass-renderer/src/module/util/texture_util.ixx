module;

#include <_pch.h>

#include "ituGL/texture/Texture2DObject.h"

export module app.util.texture;

export struct Texture2DMetadata {
    int width;
    int height;
    int components;
};

export Texture2DObject LoadTexture(const char* path, Texture2DMetadata& texture_metadata)
{
    Texture2DObject texture;

    // Load the texture data here
    unsigned char* data = stbi_load(
        path,
        &texture_metadata.width,
        &texture_metadata.height,
        &texture_metadata.components,
        4
    );

    assert(data != nullptr);

    texture.Bind();
    texture.SetImage(
        0,
        texture_metadata.width,
        texture_metadata.height,
        TextureObject::FormatRGBA,
        TextureObject::InternalFormatRGBA,
        std::span<const unsigned char>(data, texture_metadata.width * texture_metadata.height * 4)
    );

    // Release texture data
    stbi_image_free(data);

    return texture;
}

export Texture2DObject CreateDefaultTexture()
{
    Texture2DObject texture;

    constexpr auto width = 4;
    constexpr auto height = 4;

    std::vector<float> pixels;
    for (int j = 0; j < height; ++j)
    {
        for (int i = 0; i < width; ++i)
        {
            pixels.push_back(1.0f);
            pixels.push_back(0.0f);
            pixels.push_back(1.0f);
            pixels.push_back(1.0f);
        }
    }

    texture.Bind();
    texture.SetImage<float>(0, width, height, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA, pixels);
    texture.GenerateMipmap();

    return texture;
}