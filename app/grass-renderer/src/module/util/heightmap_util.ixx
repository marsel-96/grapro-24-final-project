module;

#include <_pch.h>

#include "ituGL/texture/Texture2DObject.h"

export module app.util.heightmap;

export std::shared_ptr<Texture2DObject> CreateHeightMap(
    const unsigned int width,
    const unsigned int height,
    const glm::ivec2 coords
) {
    auto heightmap = std::make_shared<Texture2DObject>();

    std::vector<float> pixels(height * width);
    for (unsigned int j = 0; j < height; ++j)
    {
        for (unsigned int i = 0; i < width; ++i)
        {
            const auto x = static_cast<float>(i) / static_cast<float>(width - 1) + static_cast<float>(coords.x);
            const auto y = static_cast<float>(j) / static_cast<float>(height - 1) + static_cast<float>(coords.y);
            pixels[j * width + i] = stb_perlin_fbm_noise3(x, y, 0.0f, 1.9f, 0.5f, 8) * 0.5f;
        }
    }

    heightmap->Bind();
    heightmap->SetImage<float>(0,
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height),
        TextureObject::FormatR,
        TextureObject::InternalFormatR16F,
        pixels);

    heightmap->GenerateMipmap();

    return heightmap;
}
