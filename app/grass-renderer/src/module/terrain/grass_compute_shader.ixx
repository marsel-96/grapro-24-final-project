module;

#include <_pch.h>

#include "itugl/asset/ModelLoader.h"
#include "ituGL/shader/ShaderUniformCollection.h"
#include "ituGL/asset/ShaderLoader.h"
#include "itugl/asset/Texture2DLoader.h"
#include "itugl/asset/TextureCubemapLoader.h"
#include "itugl/buffer/AtomicCounterBufferObject.h"
#include "itugl/buffer/SharedStorageBufferObject.h"
#include "itugl/camera/Camera.h"
#include "itugl/camera/CameraController.h"
#include "itugl/compute/Compute.h"
#include "itugl/geometry/Mesh.h"
#include "itugl/lighting/PointLight.h"
#include "itugl/renderer/ForwardRenderPass.h"
#include "itugl/renderer/Renderer.h"
#include "itugl/renderer/SkyboxRenderPass.h"
#include "itugl/scene/ImGuiSceneVisitor.h"
#include "itugl/scene/SceneModel.h"
#include "ituGL/shader/Material.h"

export module terrain.grass_compute_shader;

import app.grass_renderer_common;
import app.util.mesh;

constexpr std::array<unsigned int, 1> reset {};

export class GrassComputeShader final : public GrassRenderer {

    std::shared_ptr<Material> m_terrainMaterial;
    std::shared_ptr<Material> m_grassMaterial;

    std::shared_ptr<Texture2DObject> m_windTexture;
    std::shared_ptr<Texture2DObject> m_terrainHeightMap;
    std::shared_ptr<Texture2DObject> m_terrainTexture;
    std::shared_ptr<Texture2DObject> m_grassBladeHeightMap;

    std::shared_ptr<TextureCubemapObject> m_skyboxTexture;
    std::shared_ptr<Material> m_deferredMaterial;
    std::shared_ptr<Material> m_gBufferMaterial;

    float m_heightMapSize = 512;
    float m_heightMultiplier = 18.0f;

    float m_grassBladeMapsSize = 512.0f;
    float m_grassBladeHeightMin = 0.15f;
    float m_grassBladeHeightMax = 4.0f;
    float m_grassBladeHeightMultiplier = 6.0f;
    float m_grassBladeHeightJitter = 0.15f;
    float m_grassBaseWidth = .010;
    float m_grassWidthOffset = 0.110f;
    float m_grassBaseSideCurve = 0.5;
    float m_grassSideCurveOffset = 0.28f;
    float m_grassBaseBend = 0.39;
    float m_grassBendOffset = 0.32f;
    float m_jitterStrength = 2.0f;
    float m_windSpeed = 1.0f;

    int m_patchWidth = 128;
    int m_patchHeight = 128;

    glm::vec3 m_color = glm::vec3(1.0f);
    glm::vec3 m_bottomColor = glm::vec3(0.06f, 0.38f, 0.07f);
    glm::vec3 m_topColor = glm::vec3(0.51f, 0.549f, 0.229f);
    float m_ambientReflectance = 1.2f;
    float m_diffuseReflectance = 2.0f;
    float m_specularReflectance = 0.82f;
    float m_specularExponent = 100.0f;
    glm::vec3 m_ambientColor = glm::vec3(0.55);
    glm::vec2 m_windDirection = glm::vec2(0.5, 0.5);

    unsigned int m_resolution = 512;

    std::shared_ptr<Model> m_grassModel;

    std::shared_ptr<AtomicCounterBufferObject> m_atomicCounterBuffer;
    std::shared_ptr<SharedStorageBufferObject> m_drawIndirectBuffer;
    std::shared_ptr<SharedStorageBufferObject> m_grassBladeSSBO;

    std::shared_ptr<ComputeCall> m_grassCompute;
    std::shared_ptr<ComputeCall> m_grassInstantiatorCompute;

public:

    GrassComputeShader() = default;

private:

    void InitGrassMesh() { // NOLINT(*-convert-member-functions-to-static)
        ModelLoader modelLoader;

        m_grassModel = modelLoader.LoadShared("assets/model/grass_compute_shader/grass_blade.fbx");
        m_drawIndirectBuffer = std::make_shared<SharedStorageBufferObject>();
        m_drawIndirectBuffer->Bind();
        m_drawIndirectBuffer->AllocateData<unsigned int[5]>(1);
        m_drawIndirectBuffer->Unbind();

        m_grassModel->GetMesh().AddDrawIndirectBuffer(0, *m_drawIndirectBuffer);
    }

    void InitTextures() { // NOLINT(*-convert-member-functions-to-static)
        auto loaderRGBToRGB8 = Texture2DLoader(TextureObject::FormatRGB, TextureObject::InternalFormatRGB8);
        auto loaderRoR8 = Texture2DLoader(TextureObject::FormatR, TextureObject::InternalFormatR8);

        {
            m_skyboxTexture = TextureCubemapLoader::LoadTextureShared("assets/skybox/autumn_field.hdr", TextureObject::FormatRGB, TextureObject::InternalFormatRGB16F);
        }
        {
            m_terrainTexture = std::make_unique<Texture2DObject>(
                loaderRGBToRGB8.Load("assets/textures/dirt.png")
            );
        }
        {
            auto loader = Texture2DLoader(TextureObject::FormatR, TextureObject::InternalFormatR8);
            m_terrainHeightMap = std::make_unique<Texture2DObject>(
                loaderRoR8.Load("assets/textures/grass_compute_shader/heightmap.png")
            );
            m_terrainHeightMap->Bind();
            m_terrainHeightMap->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
            m_terrainHeightMap->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);
            m_terrainHeightMap->Unbind();
        }
        {
            m_grassBladeHeightMap = std::make_unique<Texture2DObject>(
                loaderRoR8.Load("assets/textures/grass_compute_shader/grass_height.png")
            );
            m_grassBladeHeightMap->Bind();
            m_grassBladeHeightMap->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
            m_grassBladeHeightMap->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);
            m_grassBladeHeightMap->Unbind();
        }
        {
            m_windTexture = std::make_unique<Texture2DObject>(
                loaderRoR8.Load("assets/textures/grass_compute_shader/wind.png")
            );
        }

        {
            auto loader = Texture2DLoader(TextureObject::FormatR, TextureObject::InternalFormatR8);
            m_windTexture = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/grass_compute_shader/wind.png")
            );
        }
    }

    void InitSSBOBuffer() {
        m_grassBladeSSBO = std::make_shared<SharedStorageBufferObject>();

        m_grassBladeSSBO->Bind();
        m_grassBladeSSBO->AllocateData<float[16]>(m_resolution * m_resolution);
        m_grassBladeSSBO->Unbind();
        m_grassCompute->AddBufferBinding(m_grassBladeSSBO, 0);
    }

    void InitGrassCompute() {
        const auto cs = m_computeShaderLoader.Load("shaders/grass_compute_shader/grass/grass.comp");
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

        InitSSBOBuffer();

        m_grassCompute->SetUniformValue("Resolution", m_resolution);
        m_grassCompute->SetUniformValue("GrassSpacing", m_patchWidth / static_cast<float>(m_resolution));
        m_grassCompute->SetUniformValue("PlaneCentre", glm::vec3(m_patchWidth / -2.0f, 0.0f, m_patchHeight / -2.0f));
        m_grassCompute->SetUniformValue("GrassBladeMapsSize", m_grassBladeMapsSize);
        m_grassCompute->SetUniformValue("GrassBladeHeightMap", m_grassBladeHeightMap);
        m_grassCompute->SetUniformValue("GrassBladeHeightMin", m_grassBladeHeightMin);
        m_grassCompute->SetUniformValue("GrassBladeHeightMax", m_grassBladeHeightMax);
        m_grassCompute->SetUniformValue("GrassBladeHeightMultiplier", m_grassBladeHeightMultiplier);
        m_grassCompute->SetUniformValue("GrassBladeTypeMap", m_grassBladeHeightMap);
        m_grassCompute->SetUniformValue("JitterStrength", m_jitterStrength);
        m_grassCompute->SetUniformValue("ViewProjectionMatrix", m_camera->GetViewProjectionMatrix());
        m_grassCompute->SetUniformValue("WorldMatrix", glm::mat4(1.0f));
        m_grassCompute->SetUniformValue("HeightMapTexture", m_terrainHeightMap);
        m_grassCompute->SetUniformValue("HeightMapSize", m_heightMapSize);
        m_grassCompute->SetUniformValue("HeightMultiplier", m_heightMultiplier);
        m_grassCompute->SetUniformValue("TerrainSize", glm::vec2(m_patchWidth, m_patchHeight));
        m_grassCompute->SetUniformValue("GrassBladeHeightJitter", m_grassBladeHeightJitter);
        m_grassCompute->SetUniformValue("GrassWidthOffset", m_grassWidthOffset);
        m_grassCompute->SetUniformValue("GrassSideCurveOffset", m_grassSideCurveOffset);
        m_grassCompute->SetUniformValue("GrassBaseBend", m_grassBaseBend);
        m_grassCompute->SetUniformValue("GrassBendOffset", m_grassBendOffset);
        m_grassCompute->SetUniformValue("GrassBaseWidth", m_grassBaseWidth);
        m_grassCompute->SetUniformValue("GrassBaseSideCurve", m_grassBaseSideCurve);

        m_grassCompute->AddBufferBinding(m_atomicCounterBuffer, 0);
    }

    void InitGrassInstantiatorCompute() {
        const auto cs = m_computeShaderLoader.Load("shaders/grass_compute_shader/grass/grass_instantiator.comp");
        auto shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->Build(cs);

        m_grassInstantiatorCompute = std::make_shared<ComputeCall>(shaderProgram,glm::uvec3(1,1,1));
        assert(m_grassModel);
        const auto& drawCall = m_grassModel->GetMesh().GetSubmeshDrawcall(0);

        m_grassInstantiatorCompute->SetUniformValue("FirstIndex", static_cast<unsigned int>(drawCall.GetFirst()));
        m_grassInstantiatorCompute->SetUniformValue("BaseVertex", 0u);
        m_grassInstantiatorCompute->SetUniformValue("BaseInstance", 0u);
        m_grassInstantiatorCompute->SetUniformValue("Count", static_cast<unsigned int>(drawCall.GetCount()));

        m_grassInstantiatorCompute->AddBufferBinding(m_drawIndirectBuffer, 0);
    }

    void InitGrassShader() {
        const auto vs = m_vertexShaderLoader.Load("shaders/grass_compute_shader/grass/grass.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_compute_shader/grass/grass.frag");

        auto grassShaderProgram = std::make_shared<ShaderProgram>();
        grassShaderProgram->Build(vs, fs);

        // // Terrain materials
        m_grassMaterial = std::make_unique<Material>(grassShaderProgram);
        m_grassMaterial->SetUniformValue("Color", m_color);
        m_grassMaterial->SetUniformValue("BottomColor", m_bottomColor);
        m_grassMaterial->SetUniformValue("TopColor", m_topColor);
        m_grassMaterial->SetUniformValue("AmbientReflectance", m_ambientReflectance);
        m_grassMaterial->SetUniformValue("DiffuseReflectance", m_diffuseReflectance);
        m_grassMaterial->SetUniformValue("SpecularReflectance", m_specularReflectance);
        m_grassMaterial->SetUniformValue("SpecularExponent", m_specularExponent);
        m_grassMaterial->SetUniformValue("AmbientColor", m_ambientColor);
        m_grassMaterial->SetUniformValue("WindDirection", m_windDirection);
        m_grassMaterial->SetUniformValue("WindSpeed", m_windSpeed);


        AddStandardLightUniform(grassShaderProgram);

        m_grassModel->SetMaterial(0, m_grassMaterial);
        auto& drawCall = m_grassModel->GetMesh().GetSubmeshDrawcall(0);
        drawCall.SetCommand(Drawcall::DrawCommand::Indirect);

        m_scene.AddSceneNode(std::make_shared<SceneModel>("blades", m_grassModel));

        m_grassCompute->AddBufferBinding(m_grassBladeSSBO, 0);
    }

    void InitTerrainShader() {
        const auto vs = m_vertexShaderLoader.Load("shaders/grass_compute_shader/terrain/terrain.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_compute_shader/terrain/terrain.frag");

        auto shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->Build(vs, fs);

        // This Shader needs only the world matrix and the view projection matrix
        AddMVCUniform(shaderProgram);

        // // Terrain materials
        m_terrainMaterial = std::make_unique<Material>(shaderProgram);
        m_terrainMaterial->SetUniformValue("Color", glm::vec4(1.0f));
        m_terrainMaterial->SetUniformValue("ColorTextureScale", glm::vec2(0.125f));
        m_terrainMaterial->SetUniformValue("AlbedoTexture", m_terrainTexture);
        m_terrainMaterial->SetUniformValue("HeightMapTexture", m_terrainHeightMap);
        m_terrainMaterial->SetUniformValue("HeightMapSize", m_heightMapSize);
        m_terrainMaterial->SetUniformValue("HeightMultiplier", m_heightMultiplier);
        m_terrainMaterial->SetUniformValue("TerrainSize", glm::vec2(m_patchWidth, m_patchHeight));

        const auto terrainMesh = std::make_shared<Mesh>();

        CreateTerrainMesh(*terrainMesh, m_patchWidth, m_patchHeight, 1.0);

        const auto terrain = std::make_shared<Model>(terrainMesh);
        terrain->AddMaterial(m_terrainMaterial);

        m_scene.AddSceneNode(std::make_shared<SceneModel>("terrain", terrain));
    }

    void InitDeferredMaterials() {
        // G-buffer material
        {
            // Load and build shader
            const auto vertexShader = m_vertexShaderLoader.Load("shaders/renderer/deferred/gbuffer.vert");
            const auto fragmentShader = m_fragmentShaderLoader.Load("shaders/renderer/deferred/gbuffer.frag");

            const auto shaderProgram = std::make_shared<ShaderProgram>();
            shaderProgram->Build(vertexShader, fragmentShader);

            // Get transform related uniform locations
            const auto worldViewMatrixLocation = shaderProgram->GetUniformLocation("WorldViewMatrix");
            const auto worldViewProjMatrixLocation = shaderProgram->GetUniformLocation("WorldViewProjMatrix");

            // Register shader with renderer
            m_renderer.RegisterShaderProgram(shaderProgram,
                [=](
                    const ShaderProgram &_shaderProgram,
                    const glm::mat4 &worldMatrix,
                    const Camera &camera,
                    bool cameraChanged) {
                    _shaderProgram.SetUniform(worldViewMatrixLocation, camera.GetViewMatrix() * worldMatrix);
                    _shaderProgram.SetUniform(worldViewProjMatrixLocation, camera.GetViewProjectionMatrix() * worldMatrix);
                }, nullptr
            );

            // Filter out uniforms that are not material properties
            ShaderUniformCollection::NameSet filteredUniforms;

            filteredUniforms.insert("WorldViewMatrix");
            filteredUniforms.insert("WorldViewProjMatrix");

            // Create material
            m_gBufferMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);
        }

        // Deferred material
        {
            const auto vertexShader = ShaderLoader(Shader::VertexShader).Load("shaders/renderer/deferred/deferred.vert");
            const auto fragmentShader = ShaderLoader(Shader::FragmentShader).Load("shaders/renderer/deferred/deferred.frag");

            const auto shaderProgram = std::make_shared<ShaderProgram>();
            shaderProgram->Build(vertexShader, fragmentShader);

            // Filter out uniforms that are not material properties
            ShaderUniformCollection::NameSet filteredUniforms;
            filteredUniforms.insert("InvProjMatrix");
            filteredUniforms.insert("WorldViewProjMatrix");

            // Get transform related uniform locations
            const auto invViewMatrixLocation = shaderProgram->GetUniformLocation("InvViewMatrix");
            const auto invProjMatrixLocation = shaderProgram->GetUniformLocation("InvProjMatrix");
            const auto worldViewProjMatrixLocation = shaderProgram->GetUniformLocation("WorldViewProjMatrix");

            // Register shader with renderer
            m_renderer.RegisterShaderProgram(shaderProgram,
            [=](
                const ShaderProgram &_shaderProgram,
                const glm::mat4 &worldMatrix,
                const Camera &camera,
                bool cameraChanged
                ) {
                    if (cameraChanged) {
                        _shaderProgram.SetUniform(invViewMatrixLocation, inverse(camera.GetViewMatrix()));
                        _shaderProgram.SetUniform(invProjMatrixLocation, inverse(camera.GetProjectionMatrix()));
                    }
                    _shaderProgram.SetUniform(worldViewProjMatrixLocation, camera.GetViewProjectionMatrix() * worldMatrix);
                }, GetUpdateLightsFunction(shaderProgram)
            );

            // Create material
            m_deferredMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);
        }
    }

    [[nodiscard]] auto GetUpdateLightsFunction(const std::shared_ptr<ShaderProgram>& shaderProgram) const
    {
        // Get lighting related uniform locations
        const auto ambientColorLocation = shaderProgram->GetUniformLocation("AmbientColor");
        const auto lightColorLocation = shaderProgram->GetUniformLocation("LightColor");
        const auto lightPositionLocation = shaderProgram->GetUniformLocation("LightPosition");
        const auto lightDirectionLocation = shaderProgram->GetUniformLocation("LightDirection");
        const auto lightAttenuationLocation = shaderProgram->GetUniformLocation("LightAttenuation");

        return [=](const ShaderProgram& _shaderProgram, std::span<const Light* const> lights, unsigned int& lightIndex) -> bool
        {
            bool needsRender = false;

            if (lightIndex == 0)
            {
                _shaderProgram.SetUniform(ambientColorLocation, m_ambientColor);
                needsRender = true;
            }
            else
            {
                _shaderProgram.SetUniform(ambientColorLocation, glm::vec3(0));
            }

            if (lightIndex < lights.size())
            {
                const Light& light = *lights[lightIndex];
                _shaderProgram.SetUniform(lightColorLocation, light.GetColor() * light.GetIntensity());
                _shaderProgram.SetUniform(lightPositionLocation, light.GetPosition());
                _shaderProgram.SetUniform(lightDirectionLocation, light.GetDirection());
                _shaderProgram.SetUniform(lightAttenuationLocation, light.GetAttenuation());
                needsRender = true;
            }
            else
            {
                // Disable light
                _shaderProgram.SetUniform(lightColorLocation, glm::vec3(0.0f));
            }

            lightIndex++;

            return needsRender;
        };
    }

    void InitRenderer() {
        m_renderer.AddRenderPass(std::make_unique<SkyboxRenderPass>(m_skyboxTexture));
        m_renderer.AddRenderPass(std::make_unique<ForwardRenderPass>());
    }

public:

    void Initialize() override {
        GrassRenderer::Initialize();

        InitCamera(
            glm::vec3(-20, 70, 20),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)
        );

        InitLights();
        InitGrassMesh();
        InitTextures();

        // InitDeferredMaterials();
        InitTerrainShader();
        InitGrassCompute();
        InitGrassInstantiatorCompute();
        InitGrassShader();

        InitRenderer();
    }

    void Update() override {
        GrassRenderer::Update();
    }

    void Render() override {
        m_grassCompute->Compute();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        m_grassInstantiatorCompute->Compute();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        m_grassBladeSSBO->BindToIndex(0);

        GrassRenderer::Render();
    }

    void RenderGUI() override{
        m_imGui.BeginFrame();

        // Draw GUI for scene nodes, using the visitor pattern
        ImGuiSceneVisitor imGuiVisitor(m_imGui, "Scene");
        m_scene.AcceptVisitor(imGuiVisitor);

        // Draw GUI for camera controller
        m_cameraController.DrawGUI(m_imGui);

        if (auto window = m_imGui.UseWindow("Grass Controls"))
        {
            if (m_grassMaterial)
            {
                ImGui::Separator();
                ImGui::Text("Grass Placement Properties");

                constexpr unsigned int min = 32, max = 2048;
                if (ImGui::DragScalar("Resolution", ImGuiDataType_U32, &m_resolution, 16u, &min, &max)) {
                    m_grassCompute->SetUniformValue("Resolution", m_resolution);
                    m_grassCompute->SetUniformValue("GrassSpacing", m_patchWidth / static_cast<float>(m_resolution));
                    m_grassCompute->ChangeWorkGroupSize(glm::uvec3(m_resolution / 32, m_resolution / 32, 1));
                    InitSSBOBuffer();
                }
                if (ImGui::DragFloat("Grass Blade Height Multiplier", &m_grassBladeHeightMultiplier, 0.1f, 0.00f, 15.0f)) {
                    m_grassCompute->SetUniformValue("GrassBladeHeightMultiplier", m_grassBladeHeightMultiplier);
                }
                if (ImGui::DragFloat("Grass Blade Height Min", &m_grassBladeHeightMin, 0.01f, 0.00f, 1.0f))
                {
                    m_grassCompute->SetUniformValue("GrassBladeHeightMin", m_grassBladeHeightMin);
                }
                if (ImGui::DragFloat("Grass Blade Height Max", &m_grassBladeHeightMax, 0.1f, 0.00f, 15.0f))
                {
                    m_grassCompute->SetUniformValue("GrassBladeHeightMax", m_grassBladeHeightMax);
                }
                if (ImGui::DragFloat("Grass Blade Height Jitter", &m_grassBladeHeightJitter, 0.01f, 0.0f, 0.5f))
                {
                    m_grassCompute->SetUniformValue("GrassBladeHeightJitter", m_grassBladeHeightJitter);
                }
                if (ImGui::DragFloat("Grass Blade Base Width", &m_grassBaseWidth, 0.01f, 0.0f, 1.0f))
                {
                    m_grassCompute->SetUniformValue("GrassBaseWidth", m_grassBaseWidth);
                }
                if (ImGui::DragFloat("Grass Blade Width Offset", &m_grassWidthOffset, 0.01f, 0.0f, 0.5f))
                {
                    m_grassCompute->SetUniformValue("GrassWidthOffset", m_grassWidthOffset);
                }
                if (ImGui::DragFloat("Grass Blade Base Side Curve", &m_grassBaseSideCurve, 0.01f, 0.0f, 1.0f))
                {
                    m_grassCompute->SetUniformValue("GrassBaseSideCurve", m_grassBaseSideCurve);
                }
                if (ImGui::DragFloat("Grass Blade Side Curve Offset", &m_grassSideCurveOffset, 0.01f, 0.0f, 0.5f))
                {
                    m_grassCompute->SetUniformValue("GrassSideCurveOffset", m_grassSideCurveOffset);
                }
                if (ImGui::DragFloat("Grass Blade Base Bend", &m_grassBaseBend, 0.01f, 0.0f, 2.0f))
                {
                    m_grassCompute->SetUniformValue("GrassBaseBend", m_grassBaseBend);
                }
                if (ImGui::DragFloat("Grass Blade Bend Offset", &m_grassBendOffset, 0.01f, 0.0f, 1.0f))
                {
                    m_grassCompute->SetUniformValue("GrassBendOffset", m_grassBendOffset);
                }
                if (ImGui::DragFloat("Jitter Strength", &m_jitterStrength, 0.1f, 0.0f, 10.0f))
                {
                    m_grassCompute->SetUniformValue("JitterStrength", m_jitterStrength);
                }

                ImGui::Separator();
                ImGui::Text("Wind Properties (TBD)");

                if (ImGui::DragFloat("WindSpeed", &m_windSpeed, 0.01f, 0.0f, 10.0f))
                {
                    m_grassMaterial->SetUniformValue("WindSpeed", m_windSpeed);
                }
                if (ImGui::DragFloat2("WindDirection", &m_windDirection[0], 0.01f, -1.0f, 1.0f))
                {
                    m_grassMaterial->SetUniformValue("WindDirection", m_windDirection);
                }


                ImGui::Separator();

                ImGui::Text("Grass Color & Light Properties");

                if (ImGui::ColorEdit3("Color", &m_color[0]))
                {
                    m_grassMaterial->SetUniformValue("Color", m_color);
                }
                if (ImGui::ColorEdit3("Bottom Color", &m_bottomColor[0]))
                {
                    m_grassMaterial->SetUniformValue("BottomColor", m_bottomColor);
                }
                if (ImGui::ColorEdit3("Top Color", &m_topColor[0]))
                {
                    m_grassMaterial->SetUniformValue("TopColor", m_topColor);
                }
                if (ImGui::ColorEdit3("Ambient Color", &m_ambientColor[0]))
                {
                    m_grassMaterial->SetUniformValue("AmbientColor", m_ambientColor);
                }
                if (ImGui::DragFloat("Ambient Reflectance", &m_ambientReflectance, 0.01f, 0.00f, 4.0f))
                {
                    m_grassMaterial->SetUniformValue("AmbientReflectance", m_ambientReflectance);
                }
                if (ImGui::DragFloat("Diffuse Reflectance", &m_diffuseReflectance, 0.01f, 0.00f, 4.0f))
                {
                    m_grassMaterial->SetUniformValue("DiffuseReflectance", m_diffuseReflectance);
                }
                if (ImGui::DragFloat("Specular Reflectance", &m_specularReflectance, 0.01f, 0.00f, 4.0f))
                {
                    m_grassMaterial->SetUniformValue("SpecularReflectance", m_specularReflectance);
                }
                if (ImGui::DragFloat("Specular Exponent", &m_specularExponent, 0.5f, 0.00f, 200.0f))
                {
                    m_grassMaterial->SetUniformValue("SpecularExponent", m_specularExponent);
                }

                ImGui::Separator();

            }
        }

        m_imGui.EndFrame();
    }

};
