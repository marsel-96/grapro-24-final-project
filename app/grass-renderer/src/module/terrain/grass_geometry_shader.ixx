module;

#include <_pch.h>

#include "itugl/application/Application.h"
#include "itugl/application/Window.h"
#include "itugl/asset/ModelLoader.h"
#include "ituGL/shader/ShaderUniformCollection.h"
#include "itugl/asset/TextureCubemapLoader.h"
#include "ituGL/asset/ShaderLoader.h"
#include "itugl/asset/Texture2DLoader.h"
#include "itugl/geometry/Mesh.h"
#include "itugl/geometry/Model.h"
#include "ituGL/geometry/VertexFormat.h"
#include "itugl/renderer/ForwardRenderPass.h"
#include "itugl/scene/SceneModel.h"
#include "ituGL/shader/Material.h"
#include "itugl/texture/TextureCubemapObject.h"
#include "itugl/renderer/SkyboxRenderPass.h"
#include "itugl/scene/ImGuiSceneVisitor.h"

export module terrain.grass_geometry_shader;

import app.util.mesh;
import app.grass_renderer_common;

struct GrassMeshVertex {
    GrassMeshVertex() = default;

    GrassMeshVertex(
        const glm::vec3 &position,
        const glm::vec3 &normal,
        const glm::vec3 &tangent,
        const glm::vec2 &uv
    ): position(position), normal(normal), tangent(tangent), uv(uv) {}

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 uv;
};

void CreateTerrainMeshPatch(
    Mesh &mesh,
    const unsigned int width,
    const unsigned int height,
    const unsigned int resolution
) {
    // Define the vertex format (should match the vertex structure)
    VertexFormat vertexFormat;

    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(2);

    std::vector<GrassMeshVertex> vertices;
    std::vector<unsigned int> indices;

    const auto f_res = static_cast<float>(resolution);
    const auto f_width = static_cast<float>(width);
    const auto f_height = static_cast<float>(height);

    // Iterate over each VERTEX
    for (unsigned int j = 0; j < resolution; ++j) {
        for (unsigned int i = 0; i < resolution; ++i) {

            const auto f_i = static_cast<float>(i);
            const auto f_j = static_cast<float>(j);

            // Vertex data for this vertex only
            glm::vec3 position(
                -f_width / 2.0f + f_width * f_i / f_res,
                1.0f,
                -f_height / 2.0f + f_height * f_j / f_res
            );

            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            glm::vec3 tangent(1.0f, 0.0f, 0.0f);
            glm::vec2 uv(i, j);

            vertices.emplace_back(position, normal, tangent, uv);

            // Index data for quad formed by previous vertices and current
            if (i > 0 && j > 0) {
                unsigned int top_right = j * resolution + i; // Current vertex
                unsigned int top_left = top_right - 1;
                unsigned int bottom_right = top_right - resolution;
                unsigned int bottom_left = bottom_right - 1;

                //Triangle 1
                indices.push_back(bottom_left);
                indices.push_back(bottom_right);
                indices.push_back(top_left);

                //Triangle 2
                indices.push_back(bottom_right);
                indices.push_back(top_left);
                indices.push_back(top_right);
            }
        }
    }

    glPatchParameteri(GL_PATCH_VERTICES, 3);

    mesh.AddSubmesh<GrassMeshVertex, unsigned int, VertexFormat::LayoutIterator>(
        Drawcall::Primitive::Patches,
        vertices,
        indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true),
        vertexFormat.LayoutEnd());
}

export class GrassGeometryShader final: public GrassRenderer  {

    std::shared_ptr<Material> m_terrainMaterial;
    std::shared_ptr<Material> m_grassMaterial;

    std::shared_ptr<Texture2DObject> m_grassWindDistorsionMap;
    std::shared_ptr<Texture2DObject> m_terrainTexture;
    std::shared_ptr<TextureCubemapObject> m_skyboxTexture;

    std::shared_ptr<Mesh> m_grassMesh;
    std::shared_ptr<Mesh> m_terrainMesh;

    float m_bladeForward = 0.38;
    float m_bladeCurvature = 2;
    glm::vec4 m_color = glm::vec4(1.0f);
    glm::vec4 m_bottomColor = glm::vec4(0.06f, 0.38f, 0.07f, 1.0f);
    glm::vec4 m_topColor = glm::vec4(0.56f, 0.83f, 0.32f, 1.0f);
    float m_bendRotation = 0.25f;
    float m_bladeWidth = 0.15f;
    float m_bladeWidthRandom = 0.05f;
    float m_bladeHeight = 2.85f;
    float m_bladeHeightRandom = 0.8f;
    glm::vec4 m_windFrequency = glm::vec4(0.025, 0.025, 0, 0);
    float m_windStrength = 1.0f;
    glm::vec4 m_windDistortionMapScaleOffset = glm::vec4(0.01f, 0.01f, 0.0f, 0.0f);
    glm::vec4 m_tassellationLevelOuter = glm::vec4(8.0f);
    glm::vec2 m_tassellationLevelInner = glm::vec2(8.0f);

public:
    GrassGeometryShader() = default;

private:

    void InitTextures() {
        auto loaderRGBToRGB8 = Texture2DLoader(TextureObject::FormatRGB, TextureObject::InternalFormatRGB8);
        auto loaderRoR8 = Texture2DLoader(TextureObject::FormatR, TextureObject::InternalFormatR8);

        {
            m_skyboxTexture = TextureCubemapLoader::LoadTextureShared("assets/skybox/autumn_field.hdr", TextureObject::FormatRGB, TextureObject::InternalFormatRGB16F);
        }
        {
            auto loader = Texture2DLoader(TextureObject::FormatRGB, TextureObject::InternalFormatRGB8);
            m_terrainTexture = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/dirt.png")
            );
        }
        {
            auto loader = Texture2DLoader(TextureObject::FormatRG, TextureObject::InternalFormatRG8);
            m_grassWindDistorsionMap = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/wind.png")
            );
        }
    }

    void InitGrassShader() {

        m_grassMesh = std::make_shared<Mesh>();
        CreateTerrainMeshPatch(*m_grassMesh, 512, 512, 128);

        const auto vs = m_vertexShaderLoader.Load("shaders/grass_geometry_shader/grass/grass.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_geometry_shader/grass/grass.frag");
        const auto tcs = m_tassellationControlShaderLoader.Load("shaders/grass_geometry_shader/grass/grass.tesc");
        const auto tes = m_tassellationEvaluationShaderLoader.Load("shaders/grass_geometry_shader/grass/grass.tese");
        const auto gs = m_geometryShaderLoader.Load("shaders/grass_geometry_shader/grass/grass.geom");

        auto grassShaderProgram = std::make_shared<ShaderProgram>();
        grassShaderProgram->Build(vs, fs, &tcs, tes, gs);

        AddStandardLightUniform(grassShaderProgram);

        // // Terrain materials
        m_grassMaterial = std::make_unique<Material>(grassShaderProgram);
        m_grassMaterial->SetUniformValue("Color", m_color);
        m_grassMaterial->SetUniformValue("BottomColor", m_bottomColor);
        m_grassMaterial->SetUniformValue("TopColor", m_topColor);
        m_grassMaterial->SetUniformValue("BendRotation", m_bendRotation);
        m_grassMaterial->SetUniformValue("BladeWidth", m_bladeWidth);
        m_grassMaterial->SetUniformValue("BladeWidthRandom", m_bladeWidthRandom);
        m_grassMaterial->SetUniformValue("BladeHeight", m_bladeHeight);
        m_grassMaterial->SetUniformValue("BladeHeightRandom", m_bladeHeightRandom);
        m_grassMaterial->SetUniformValue("TessellationLevelOuter", m_tassellationLevelOuter);
        m_grassMaterial->SetUniformValue("TessellationLevelInner", m_tassellationLevelInner);
        m_grassMaterial->SetUniformValue("WindDistortionMap", m_grassWindDistorsionMap);
        m_grassMaterial->SetUniformValue("WindFrequency", m_windFrequency);
        m_grassMaterial->SetUniformValue("WindStrength", m_windStrength);
        m_grassMaterial->SetUniformValue("WindDistortionMapScaleOffset", m_windDistortionMapScaleOffset);
        m_grassMaterial->SetUniformValue("BladeForward", m_bladeForward);
        m_grassMaterial->SetUniformValue("BladeCurvature", m_bladeCurvature);

        const auto grassModel = std::make_shared<Model>(m_grassMesh);
        grassModel->AddMaterial(m_grassMaterial);

        m_scene.AddSceneNode(std::make_shared<SceneModel>("grass", grassModel));
    }

    void InitTerrainShader() {
        m_terrainMesh = std::make_shared<Mesh>();
        CreateTerrainMesh(*m_terrainMesh, 512, 512, 1.0);

        const auto vs = m_vertexShaderLoader.Load("shaders/grass_geometry_shader/terrain/terrain.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_geometry_shader/terrain/terrain.frag");

        const auto& shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->Build(vs, fs);

        // // Terrain materials
        m_terrainMaterial = std::make_unique<Material>(shaderProgram);
        m_terrainMaterial->SetUniformValue("Color", glm::vec4(1.0f));
        m_terrainMaterial->SetUniformValue("ColorTextureScale", glm::vec2(0.125f));
        m_terrainMaterial->SetUniformValue("AlbedoTexture", m_terrainTexture);

        AddMVCUniform(shaderProgram);

        const auto terrain = std::make_shared<Model>(m_terrainMesh);
        terrain->AddMaterial(m_terrainMaterial);

        m_scene.AddSceneNode(std::make_shared<SceneModel>("terrain", terrain));
    }

    void InitRenderer() {
        m_renderer.AddRenderPass(std::make_unique<ForwardRenderPass>());
        m_renderer.AddRenderPass(std::make_unique<SkyboxRenderPass>(m_skyboxTexture));
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
        InitTextures();
        InitGrassShader();
        InitTerrainShader();
        InitRenderer();
    }

    void Update() override {
        GrassRenderer::Update();
    }

    void Render() override {
        GrassRenderer::Render();
    }

    void RenderGUI() override {
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
                ImGui::Text("Tassellation Properties");

                if (ImGui::DragFloat4("Tessellation Level Outer", &m_tassellationLevelOuter[0], 1.0f, 0.00f, 32.0f))
                {
                    m_grassMaterial->SetUniformValue("TessellationLevelOuter", m_tassellationLevelOuter);
                }
                if (ImGui::DragFloat2("Tessellation Level Inner", &m_tassellationLevelInner[0], 1.0f, 0.00f, 32.0f))
                {
                    m_grassMaterial->SetUniformValue("TessellationLevelInner", m_tassellationLevelInner);
                }

                ImGui::Separator();
                ImGui::Text("Grass Placement Properties");

                if (ImGui::DragFloat("Grass Blade Width", &m_bladeWidth, 0.01f, 0.00f, 2.0f))
                {
                    m_grassMaterial->SetUniformValue("BladeWidth", m_bladeWidth);
                }
                if (ImGui::DragFloat("Grass Blade Width Offset", &m_bladeWidthRandom, 0.01f, 0.00f, 1.0f))
                {
                    m_grassMaterial->SetUniformValue("BladeWidthRandom", m_bladeWidthRandom);
                }
                if (ImGui::DragFloat("Grass Blade Height", &m_bladeHeight, 0.1f, 0.01f, 10.0f))
                {
                    m_grassMaterial->SetUniformValue("BladeHeight", m_bladeHeight);
                }
                if (ImGui::DragFloat("Grass Blade Height Offset", &m_bladeHeightRandom, 0.05f, 0.00f, 5.0f))
                {
                    m_grassMaterial->SetUniformValue("BladeHeightRandom", m_bladeHeightRandom);
                }
                if (ImGui::DragFloat("Grass Bend Rotation", &m_bendRotation, 0.01f, 0.00f, 10.0f))
                {
                    m_grassMaterial->SetUniformValue("BendRotation", m_bendRotation);
                }
                if (ImGui::DragFloat("Grass Blade Forward", &m_bladeForward, 0.01f, 0.00f, 10.0f))
                {
                    m_grassMaterial->SetUniformValue("BladeForward", m_bladeForward);
                }
                if (ImGui::DragFloat("Grass Blade Curvature", &m_bladeCurvature, 0.01f, 0.00f, 10.0f))
                {
                    m_grassMaterial->SetUniformValue("BladeCurvature", m_bladeCurvature);
                }

                ImGui::Separator();
                ImGui::Text("Wind Properties (TBD)");

                if (ImGui::DragFloat("Wind Strength", &m_windStrength, 0.01f, 0.00f, 10.0f))
                {
                    m_grassMaterial->SetUniformValue("WindStrength", m_windStrength);
                }
                if (ImGui::DragFloat2("Wind Frequency", &m_windFrequency[0], 1.0f, 0.00f, 32.0f))
                {
                    m_grassMaterial->SetUniformValue("WindFrequency", m_windFrequency);
                }

                ImGui::Text("Grass Color & Light Properties");
                ImGui::Separator();

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

            }
        }

        m_imGui.EndFrame();
    }



};
