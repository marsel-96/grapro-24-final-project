module;

#include <_pch.h>

#include "Scene.h"
#include "itugl/application/Window.h"
#include "ituGL/shader/ShaderUniformCollection.h"

#include "ituGL/asset/ShaderLoader.h"
#include "itugl/asset/Texture2DLoader.h"
#include "itugl/geometry/Mesh.h"
#include "ituGL/shader/Material.h"

export module terrain.heightmap_gpu;

import app.util.mesh;
import app.util.texture;

import app.camera;

export class TerrainHeightmapGPU final : public Scene {

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;
    ShaderLoader m_tassellationControlShaderLoader;
    ShaderLoader m_tassellationEvaluationShaderLoader;

    ManagedCamera m_camera;

    std::shared_ptr<Material> m_terrainMaterial;
    std::shared_ptr<Texture2DObject> m_terrainTexture;
    std::shared_ptr<Texture2DObject> m_heightmapTexture;

    std::shared_ptr<Mesh> m_terrainMesh;

public:
    TerrainHeightmapGPU()
        : m_vertexShaderLoader(Shader::VertexShader),
          m_fragmentShaderLoader(Shader::FragmentShader),
          m_tassellationControlShaderLoader(Shader::TesselationControlShader),
          m_tassellationEvaluationShaderLoader(Shader::TesselationEvaluationShader),
          m_camera(
              20.0f,
              0.5f,
              glm::vec3(10.0f, 20.0f, 10.0f),
              glm::vec3(0.0f, 0.0f, 0.0f)
          ) {
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

    void Initialize(const Window& window) override {

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

        m_camera.Initialize(window);
    }

    void Update(const Window& window, const float deltaTime) override {
        m_camera.UpdateCamera(window, deltaTime);
    }

    void Render() override {
        const auto& material = *m_terrainMaterial;
        const auto& shaderProgram = *material.GetShaderProgram();
        const auto& mesh = *m_terrainMesh;
        const auto& worldMatrix = glm::mat4(1.0f);

        material.Use();

        const auto locationWorldMatrix = shaderProgram.GetUniformLocation("WorldMatrix");
        const auto locationViewProjMatrix = shaderProgram.GetUniformLocation("ViewProjMatrix");

        material.GetShaderProgram()->SetUniform(locationWorldMatrix, worldMatrix);
        material.GetShaderProgram()->SetUniform(locationViewProjMatrix, m_camera.GetViewProjectionMatrix());

        mesh.DrawSubmesh(0);
    }

};
