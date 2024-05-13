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

export module terrain.grass_compute_shader_3;

import app.util.texture;
import app.util.mesh;
import app.camera;

constexpr std::array<unsigned int, 1> reset {};

export class GrassComputeShader3 final : public Scene {

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
    std::shared_ptr<Texture2DObject> m_grassBladeHeightMap;

    float m_heightMapSize = 512;
    float m_heightMultiplier = 18.0f;

    float m_grassBladeMapsSize = 512.0f;
    float m_grassBladeHeightMin = 0.15f;
    float m_grassBladeHeightMax = 5.0f;
    float m_grassBladeHeightJitter = 0.15f;

    float m_jitterStrength = 2.0f;

    int m_patchWidth = 128;
    int m_patchHeight = 128;

    unsigned int m_resolution = 768;

    std::shared_ptr<Model> m_grassModel;
    std::shared_ptr<Mesh> m_terrainMesh;

    std::shared_ptr<AtomicCounterBufferObject> m_atomicCounterBuffer;
    std::shared_ptr<SharedStorageBufferObject> m_drawIndirectBuffer;

    std::shared_ptr<ComputeCall> m_grassCompute;
    std::shared_ptr<ComputeCall> m_grassInstantiatorCompute;

public:
    explicit GrassComputeShader3(const Application &application)
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
        m_drawIndirectBuffer = std::make_shared<SharedStorageBufferObject>();
        m_drawIndirectBuffer->Bind();
        m_drawIndirectBuffer->AllocateData<unsigned int[5]>(1);
        m_drawIndirectBuffer->Unbind();

        m_grassModel->GetMesh().AddDrawIndirectBuffer(0, *m_drawIndirectBuffer);
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
                loader.Load("assets/textures/grass_compute_shader_3/heightmap.png")
            );
            m_terrainHeightMap->Bind();
            m_terrainHeightMap->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
            m_terrainHeightMap->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);
            m_terrainHeightMap->Unbind();
        }
        {
            auto loader = Texture2DLoader(TextureObject::FormatR, TextureObject::InternalFormatR8);
            m_grassBladeHeightMap = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/grass_compute_shader_3/grass_height.png")
            );
            m_grassBladeHeightMap->Bind();
            m_grassBladeHeightMap->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
            m_grassBladeHeightMap->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);
            m_grassBladeHeightMap->Unbind();
        }

        {
            auto loader = Texture2DLoader(TextureObject::FormatR, TextureObject::InternalFormatR8);
            m_windTexture = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/grass_compute_shader_3/wind.png")
            );
        }

    }

    void InitGrassCompute() {
        const auto cs = m_computeShaderLoader.Load("shaders/grass_compute_shader_3/grass/grass.comp");
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
        ssboMatrices->AllocateData<float[16]>(m_resolution * m_resolution);
        ssboMatrices->Unbind();

        m_grassCompute->SetUniformValue("Resolution", m_resolution);
        m_grassCompute->SetUniformValue("GrassSpacing", m_patchWidth / static_cast<float>(m_resolution));
        m_grassCompute->SetUniformValue("PlaneCentre", glm::vec3(
                                            m_patchWidth / -2.0f,
                                            0.0f,
                                            m_patchHeight / -2.0f
                                        ));
        m_grassCompute->SetUniformValue("GrassBladeMapsSize", m_grassBladeMapsSize);
        m_grassCompute->SetUniformValue("GrassBladeHeightMap", m_grassBladeHeightMap);
        m_grassCompute->SetUniformValue("GrassBladeHeightMin", m_grassBladeHeightMin);
        m_grassCompute->SetUniformValue("GrassBladeHeightMax", m_grassBladeHeightMax);
        m_grassCompute->SetUniformValue("GrassBladeTypeMap", m_grassBladeHeightMap);
        m_grassCompute->SetUniformValue("JitterStrength", m_jitterStrength);
        m_grassCompute->SetUniformValue("ViewProjectionMatrix", m_camera.GetViewProjectionMatrix());
        m_grassCompute->SetUniformValue("WorldMatrix", glm::mat4(1.0f));
        m_grassCompute->SetUniformValue("HeightMapTexture", m_terrainHeightMap);
        m_grassCompute->SetUniformValue("HeightMapSize", m_heightMapSize);
        m_grassCompute->SetUniformValue("HeightMultiplier", m_heightMultiplier);
        m_grassCompute->SetUniformValue("TerrainSize", glm::vec2(m_patchWidth, m_patchHeight));
        m_grassCompute->SetUniformValue("GrassBladeHeightJitter", m_grassBladeHeightJitter);

        m_grassCompute->AddBufferBinding(ssboMatrices, 0);
        m_grassCompute->AddBufferBinding(m_atomicCounterBuffer, 0);
    }

    void InitGrassInstantiatorCompute() {
        const auto cs = m_computeShaderLoader.Load("shaders/grass_compute_shader_3/grass/grass_instantiator.comp");
        auto shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->Build(cs);

        m_grassInstantiatorCompute = std::make_shared<ComputeCall>(shaderProgram,glm::uvec3(1,1,1));

        const auto& drawCall = m_grassModel->GetMesh().GetSubmeshDrawcall(0);

        m_grassInstantiatorCompute->SetUniformValue("FirstIndex", static_cast<unsigned int>(drawCall.GetFirst()));
        m_grassInstantiatorCompute->SetUniformValue("BaseVertex", 0u);
        m_grassInstantiatorCompute->SetUniformValue("BaseInstance", 0u);
        m_grassInstantiatorCompute->SetUniformValue("Count", static_cast<unsigned int>(drawCall.GetCount()));

        m_grassInstantiatorCompute->AddBufferBinding(m_drawIndirectBuffer, 0);
    }

    void InitGrassShader() {
        const auto vs = m_vertexShaderLoader.Load("shaders/grass_compute_shader_3/grass/grass.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_compute_shader_3/grass/grass.frag");

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

        const auto vs = m_vertexShaderLoader.Load("shaders/grass_compute_shader_3/terrain/terrain.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_compute_shader_3/terrain/terrain.frag");

        auto shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->Build(vs, fs);

        // // Terrain materials
        m_terrainMaterial = std::make_unique<Material>(shaderProgram);
        m_terrainMaterial->SetUniformValue("Color", glm::vec4(1.0f));
        m_terrainMaterial->SetUniformValue("ColorTextureScale", glm::vec2(0.125f));
        m_terrainMaterial->SetUniformValue("AlbedoTexture", m_terrainTexture);
        m_terrainMaterial->SetUniformValue("HeightMapTexture", m_terrainHeightMap);
        m_terrainMaterial->SetUniformValue("HeightMapSize", m_heightMapSize);
        m_terrainMaterial->SetUniformValue("HeightMultiplier", m_heightMultiplier);
        m_terrainMaterial->SetUniformValue("TerrainSize", glm::vec2(m_patchWidth, m_patchHeight));

    }

public:

    void Initialize(const Window &window) override {
        InitTextures();

        LoadGrassMesh();

        InitGrassShader();
        InitTerrainShader();
        InitGrassCompute();
        InitGrassInstantiatorCompute();

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
    }

    void Render() override {
        const auto &worldMatrix = glm::mat4(1.0f);

        m_grassCompute->BindBuffers();

        Render(*m_terrainMaterial, *m_terrainMesh, worldMatrix);
        Render(*m_grassMaterial, m_grassModel->GetMesh(), worldMatrix, 0, true);
    }

    void Compute() const {
        m_grassCompute->Compute();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        m_grassInstantiatorCompute->Compute();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

private:
    void Render(const Material &material, const Mesh &mesh, const glm::mat4 &worldMatrix,
                const unsigned int instances = 1, bool indirect = false) const {
        const auto &shaderProgram = *material.GetShaderProgram();

        material.Use();

        const auto locationWorldMatrix = shaderProgram.GetUniformLocation("WorldMatrix");
        const auto locationViewProjMatrix = shaderProgram.GetUniformLocation("ViewProjMatrix");

        material.GetShaderProgram()->SetUniform(locationWorldMatrix, worldMatrix);
        material.GetShaderProgram()->SetUniform(locationViewProjMatrix, m_camera.GetViewProjectionMatrix());

        if (indirect) {
            mesh.DrawIndirect(0);
        } else if (instances > 1) {
            mesh.DrawSubmesh(0, instances);
        } else {
            mesh.DrawSubmesh(0);
        }
    }
};
