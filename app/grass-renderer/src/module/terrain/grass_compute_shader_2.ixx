module;

#include <_pch.h>

#include "Scene.h"
#include "itugl/application/Application.h"
#include "itugl/application/Window.h"
#include "itugl/asset/ModelLoader.h"
#include "ituGL/shader/ShaderUniformCollection.h"
#include "ituGL/asset/ShaderLoader.h"
#include "itugl/asset/Texture2DLoader.h"
#include "itugl/buffer/AtomicCounterBufferObject.h"
#include "itugl/buffer/SharedStorageBufferObject.h"
#include "itugl/compute/Compute.h"
#include "itugl/geometry/Mesh.h"
#include "ituGL/shader/Material.h"

export module terrain.grass_compute_shader_2;

import app.util.texture;
import app.util.mesh;
import app.camera;

constexpr std::array<unsigned int, 1> reset {};

export class GrassComputeShader2 final : public Scene {
    const Application &m_application;

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;
    ShaderLoader m_tassellationControlShaderLoader;
    ShaderLoader m_tassellationEvaluationShaderLoader;
    ShaderLoader m_geometryShaderLoader;
    ShaderLoader m_computeShaderLoader;

    ManagedCamera m_camera;

    std::shared_ptr<Material> m_terrainMaterial;
    std::shared_ptr<Material> m_grassMaterial;

    std::shared_ptr<Texture2DObject> m_windTexture;
    std::shared_ptr<Texture2DObject> m_terrainHeightMap;
    std::shared_ptr<Texture2DObject> m_terrainTexture;

    std::shared_ptr<Model> m_grassModel;
    std::shared_ptr<Mesh> m_terrainMesh;

    std::shared_ptr<AtomicCounterBufferObject> m_atomicCounterBuffer;

    unsigned m_patchWidth = 128;
    unsigned m_patchHeight = 128;

    unsigned int m_resolution = 800;

    unsigned int m_instancesCount = 0;

    std::shared_ptr<ComputeCall> m_grassCompute;

public:
    explicit GrassComputeShader2(const Application &application)
        : m_application(application),
          m_vertexShaderLoader(Shader::VertexShader),
          m_fragmentShaderLoader(Shader::FragmentShader),
          m_tassellationControlShaderLoader(Shader::TesselationControlShader),
          m_tassellationEvaluationShaderLoader(Shader::TesselationEvaluationShader),
          m_geometryShaderLoader(Shader::GeometryShader),
          m_computeShaderLoader(Shader::ComputeShader),
          m_camera(
              20.0f,
              0.5f,
              glm::vec3(10.0f, 20.0f, 10.0f),
              glm::vec3(0.0f, 0.0f, 0.0f)
          ) {
    }

private:

    void LoadGrassMesh() {
        ModelLoader modelLoader;
        m_grassModel = modelLoader.LoadShared("assets/model/grass_blade.fbx");
    }

    void InitTextures() {
        {
            auto loader = Texture2DLoader(TextureObject::FormatRGB, TextureObject::InternalFormatRGB8);
            m_terrainTexture = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/dirt.png")
            );
        }
        {
            auto loader = Texture2DLoader(TextureObject::FormatR, TextureObject::InternalFormatR8);
            m_terrainHeightMap = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/grass_compute_shader_2/heightmap.png")
            );
            m_terrainHeightMap->Bind();
            m_terrainHeightMap->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
            m_terrainHeightMap->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);
            m_terrainHeightMap->Unbind();
        }

        {
            auto loader = Texture2DLoader(TextureObject::FormatR, TextureObject::InternalFormatR8);
            m_windTexture = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/grass_compute_shader_2/wind.png")
            );
        }

    }

    void InitGrassCompute() {
        m_instancesCount = m_resolution * m_resolution;

        const auto cs = m_computeShaderLoader.Load("shaders/grass_compute_shader_2/grass/grass.comp");
        auto shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->Build(cs);

        m_grassCompute = std::make_shared<ComputeCall>(shaderProgram,
            glm::uvec3(
                m_resolution / 32,
                m_resolution / 32,
                1
            )
        );

        m_atomicCounterBuffer = std::make_shared<AtomicCounterBufferObject>();
        m_atomicCounterBuffer->Bind();
        m_atomicCounterBuffer->AllocateData<unsigned int>(reset, DynamicDraw);
        m_atomicCounterBuffer->Unbind();

        const auto ssboMatrices = std::make_shared<SharedStorageBufferObject>();
        ssboMatrices->Bind();
        ssboMatrices->AllocateData<glm::vec4>(m_instancesCount);
        ssboMatrices->Unbind();

        m_grassCompute->SetUniformValue("HeightMapTexture", m_terrainHeightMap);
        m_grassCompute->SetUniformValue("WindTexture", m_windTexture);

        m_grassCompute->SetUniformValue("PlacementOffset", glm::vec2(m_resolution / m_patchWidth, m_resolution / m_patchHeight));
        m_grassCompute->SetUniformValue("InitialPos", glm::vec3(
                                            m_patchWidth / -2.0f,
                                            0.0f,
                                            m_patchHeight / -2.0f
                                        ));
        m_grassCompute->SetUniformValue("HeightMapSize", 128u);
        m_grassCompute->SetUniformValue("HeightMultiplier", 20.0f);
        m_grassCompute->SetUniformValue("OccludeHeightOffset", 1.0f);
        m_grassCompute->SetUniformValue("Time", 0.0f);
        m_grassCompute->SetUniformValue("GrassInstances", m_instancesCount);

        m_grassCompute->AddBufferBinding(ssboMatrices, 0);
        m_grassCompute->AddBufferBinding(m_atomicCounterBuffer, 0);
    }

    void InitGrassShader() {
        const auto vs = m_vertexShaderLoader.Load("shaders/grass_compute_shader_2/grass/grass.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_compute_shader_2/grass/grass.frag");

        auto grassShaderProgram = std::make_shared<ShaderProgram>();
        grassShaderProgram->Build(vs, fs);

        // // Terrain materials
        m_grassMaterial = std::make_unique<Material>(grassShaderProgram);
        m_grassMaterial->SetUniformValue("Color", glm::vec4(1.0f));
        m_grassMaterial->SetUniformValue("BottomColor", glm::vec4(0.06f, 0.38f, 0.07f, 1.0f));
        m_grassMaterial->SetUniformValue("TopColor", glm::vec4(0.56f, 0.83f, 0.32f, 1.0f));
        m_grassMaterial->SetUniformValue("Offset", 0.4f);
        m_grassMaterial->SetUniformValue("Height", 0.4f);
        m_grassMaterial->SetUniformValue("WindDirection", glm::vec2(0.5, 0.5));
        m_grassMaterial->SetUniformValue("Time", 0.0f);
        m_grassMaterial->SetUniformValue("WindNoiseScale", 1.0f);
        m_grassMaterial->SetUniformValue("NoiseOffset", -0.5f);
        m_grassMaterial->SetUniformValue("MeshDeformationLimitTop", 0.08f);
        m_grassMaterial->SetUniformValue("MeshDeformationLimitLow", 2.0f);
        m_grassMaterial->SetUniformValue("WindSpeed", 2.0f);
        m_grassMaterial->SetUniformValue("ShadingOffset", 2.0f);
        m_grassMaterial->SetUniformValue("ShadingParameter", 1.7f);
    }

    void InitTerrainShader() {

        m_terrainMesh = std::make_shared<Mesh>();
        CreateTerrainMesh(*m_terrainMesh, m_patchWidth, m_patchHeight, 1.0);

        const auto vs = m_vertexShaderLoader.Load("shaders/grass_compute_shader_2/terrain/terrain.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_compute_shader_2/terrain/terrain.frag");

        auto shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->Build(vs, fs);

        // // Terrain materials
        m_terrainMaterial = std::make_unique<Material>(shaderProgram);
        m_terrainMaterial->SetUniformValue("Color", glm::vec4(1.0f));
        m_terrainMaterial->SetUniformValue("ColorTextureScale", glm::vec2(0.125f));
        m_terrainMaterial->SetUniformValue("AlbedoTexture", m_terrainTexture);
        m_terrainMaterial->SetUniformValue("HeightMapTexture", m_terrainHeightMap);
        m_terrainMaterial->SetUniformValue("HeightMapSize", 128.0f);
        m_terrainMaterial->SetUniformValue("HeightMultiplier", 19.95f);
    }

public:

    void Initialize(const Window &window) override {
        InitTextures();

        LoadGrassMesh();

        InitGrassShader();
        InitTerrainShader();
        InitGrassCompute();

        Compute();

        m_camera.Initialize(window);
    }

    void Update(const Window &window, const float deltaTime) override {
        m_camera.UpdateCamera(window, deltaTime);

        m_grassCompute->SetUniformValue("Time", m_application.GetCurrentTime());
        m_grassMaterial->SetUniformValue("Time", m_application.GetCurrentTime());

        m_atomicCounterBuffer->Bind();
        m_atomicCounterBuffer->UpdateData<unsigned int>(reset);
        m_atomicCounterBuffer->Unbind();

        Compute();
    }

    void Render() override {
        const auto &worldMatrix = glm::mat4(1.0f);

        m_grassCompute->BindBuffers();

        Render(*m_terrainMaterial, *m_terrainMesh, worldMatrix);
        Render(*m_grassMaterial, m_grassModel->GetMesh(), worldMatrix, m_instancesCount);
    }

    void Compute() const {
        m_grassCompute->Compute();

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

private:
    void Render(const Material &material, const Mesh &mesh, const glm::mat4 &worldMatrix,
                const unsigned int instances = 1) const {
        const auto &shaderProgram = *material.GetShaderProgram();

        material.Use();

        const auto locationWorldMatrix = shaderProgram.GetUniformLocation("WorldMatrix");
        const auto locationViewProjMatrix = shaderProgram.GetUniformLocation("ViewProjMatrix");

        material.GetShaderProgram()->SetUniform(locationWorldMatrix, worldMatrix);
        material.GetShaderProgram()->SetUniform(locationViewProjMatrix, m_camera.GetViewProjectionMatrix());

        if (instances > 1) {
            mesh.DrawSubmesh(0, instances);
        } else {
            mesh.DrawSubmesh(0);
        }
    }
};
