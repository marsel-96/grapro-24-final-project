module;

#include <_pch.h>

#include "ituGL/shader/ShaderUniformCollection.h"

#include "ituGL/asset/ShaderLoader.h"
#include "itugl/asset/Texture2DLoader.h"
#include "itugl/geometry/Mesh.h"
#include "ituGL/shader/Material.h"

export module terrain.heightmap_gpu;

import app.util.mesh;
import app.util.texture;

export class TerrainHeightmapGPU {

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;
    ShaderLoader m_tassellationControlShaderLoader;
    ShaderLoader m_tassellationEvaluationShaderLoader;

    std::shared_ptr<Material> m_terrainMaterial;
    std::shared_ptr<Texture2DObject> m_terrainTexture;
    std::shared_ptr<Texture2DObject> m_heightmapTexture;

    std::shared_ptr<Mesh> m_terrainMesh;

public:
    TerrainHeightmapGPU()
        : m_vertexShaderLoader(Shader::VertexShader),
          m_fragmentShaderLoader(Shader::FragmentShader),
          m_tassellationControlShaderLoader(Shader::TesselationControlShader),
          m_tassellationEvaluationShaderLoader(Shader::TesselationEvaluationShader) {
    }

private:

    void InitShader() {
        // // Terrain shader program
        const auto vs = m_vertexShaderLoader.Load("shaders/terrain_heightmap_gpu/terrain.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/terrain_heightmap_gpu/terrain.frag");
        const auto tcs = m_tassellationControlShaderLoader.Load("shaders/terrain_heightmap_gpu/terrain.tesc");
        const auto tes = m_tassellationEvaluationShaderLoader.Load("shaders/terrain_heightmap_gpu/terrain.tese");

        //
        auto terrainShaderProgram = std::make_shared<ShaderProgram>();
        terrainShaderProgram->Build(vs, fs, &tcs, tes);

        //
        // // Terrain materials
        m_terrainMaterial = std::make_unique<Material>(terrainShaderProgram);
        m_terrainMaterial->SetUniformValue("Color", glm::vec4(1.0f));
        m_terrainMaterial->SetUniformValue("Heightmap", m_heightmapTexture);
    }

public:

    void Initialize() {

        {
            auto loader = Texture2DLoader(TextureObject::FormatR, TextureObject::InternalFormatR8);
            m_heightmapTexture = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/iceland_heightmap.png")
            );
        }

        {
            auto loader = Texture2DLoader(TextureObject::FormatRGB, TextureObject::InternalFormatRGB);
            m_terrainTexture = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/dirt.png")
            );
        }

        const auto size = m_heightmapTexture->GetSize();
        m_terrainMesh = std::make_shared<Mesh>();
        CreateTerrainMeshPatch(*m_terrainMesh, size.x, size.y, 20);

        InitShader();
    }

    [[nodiscard]] const Material& GetMaterial() const {
        return *m_terrainMaterial;
    }

    [[nodiscard]] const Mesh& GetMesh() const {
        return *m_terrainMesh;
    }

};
