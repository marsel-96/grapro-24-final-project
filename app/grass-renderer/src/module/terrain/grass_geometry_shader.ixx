module;

#include <_pch.h>

#include "Scene.h"
#include "itugl/application/Application.h"
#include "itugl/application/Window.h"
#include "ituGL/shader/ShaderUniformCollection.h"

#include "ituGL/asset/ShaderLoader.h"
#include "itugl/asset/Texture2DLoader.h"
#include "itugl/geometry/Mesh.h"
#include "ituGL/geometry/VertexFormat.h"
#include "ituGL/shader/Material.h"

export module terrain.grass_geometry_shader;

import app.util.texture;
import app.util.mesh;
import app.camera;

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

void CreateTerrainMeshPatch1(
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

void CreateTerrainMeshPatch2(
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

    for (unsigned i = 0; i <= resolution - 1; i++) {
        for (unsigned j = 0; j <= resolution - 1; j++) {
            const auto f_i = static_cast<float>(i);
            const auto f_j = static_cast<float>(j);

            vertices.emplace_back(
                glm::vec3(
                    -f_width / 2.0f + f_width * f_i / f_res,
                    1.0f,
                    -f_height / 2.0f + f_height * f_j / f_res
                ),
                glm::vec3(0.0, 1.0, 0.0),
                glm::vec3(1.0, 0.0, 0.0),
                glm::vec2(f_i / f_res, f_j / f_res)
            );
            vertices.emplace_back(
                glm::vec3(
                    -f_width / 2.0f + f_width * (f_i + 1.0f) / f_res,
                    1.0f,
                    -f_height / 2.0f + f_height * f_j / f_res
                ),
                glm::vec3(0.0, 1.0, 0.0),
                glm::vec3(1.0, 0.0, 0.0),
                glm::vec2((f_i + 1.0f) / f_res, f_j / f_res)
            );
            vertices.emplace_back(
                glm::vec3(
                    -f_width / 2.0f + f_width * f_i / f_res,
                    1.0f,
                    -f_height / 2.0f + f_height * (f_j + 1.0f) / f_res
                ),
                glm::vec3(0.0, 1.0, 0.0),
                glm::vec3(1.0, 0.0, 0.0),
                glm::vec2(f_i / f_res, (f_j + 1.0f) / f_res)
            );
            vertices.emplace_back(
                glm::vec3(
                    -f_width / 2.0f + f_width * (f_i + 1.0f) / f_res,
                    1.0f,
                    -f_height / 2.0f + f_height * (f_j + 1.0f) / f_res
                ),
                glm::vec3(0.0, 1.0, 0.0),
                glm::vec3(1.0, 0.0, 0.0),
                glm::vec2((f_i + 1.0f) / f_res, (f_j + 1.0f) / f_res)
            );
        }
    }

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    mesh.AddSubmesh<GrassMeshVertex, VertexFormat::LayoutIterator>(
        Drawcall::Primitive::Patches,
        vertices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true),
        vertexFormat.LayoutEnd()
    );
}

export class GrassGeometryShader final : public Scene {

    const Application& m_application;

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;
    ShaderLoader m_tassellationControlShaderLoader;
    ShaderLoader m_tassellationEvaluationShaderLoader;
    ShaderLoader m_geometryShaderLoader;

    ManagedCamera m_camera;

    std::shared_ptr<Material> m_terrainMaterial;
    std::shared_ptr<Material> m_grassMaterial;

    std::shared_ptr<Texture2DObject> m_grassWindDistorsionMap;
    std::shared_ptr<Texture2DObject> m_terrainTexture;

    std::shared_ptr<Mesh> m_grassMesh;
    std::shared_ptr<Mesh> m_terrainMesh;

public:
    explicit GrassGeometryShader(const Application& application)
        : m_application(application),
          m_vertexShaderLoader(Shader::VertexShader),
          m_fragmentShaderLoader(Shader::FragmentShader),
          m_tassellationControlShaderLoader(Shader::TesselationControlShader),
          m_tassellationEvaluationShaderLoader(Shader::TesselationEvaluationShader),
          m_geometryShaderLoader(Shader::GeometryShader),
          m_camera(
              20.0f,
              0.5f,
              glm::vec3(10.0f, 20.0f, 10.0f),
              glm::vec3(0.0f, 0.0f, 0.0f)
          ) {
    }

private:

    void InitGrassShader() {
        {
            auto loader = Texture2DLoader(TextureObject::FormatRG, TextureObject::InternalFormatRG8);
            m_grassWindDistorsionMap = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/wind.png")
            );
        }

        m_grassMesh = std::make_shared<Mesh>();
        CreateTerrainMeshPatch1(*m_grassMesh, 512, 512, 128);

        const auto vs = m_vertexShaderLoader.Load("shaders/grass_geometry_shader/grass/grass.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_geometry_shader/grass/grass.frag");
        const auto tcs = m_tassellationControlShaderLoader.Load("shaders/grass_geometry_shader/grass/grass.tesc");
        const auto tes = m_tassellationEvaluationShaderLoader.Load("shaders/grass_geometry_shader/grass/grass.tese");
        std::vector gsSource = {
            "shaders/grass_geometry_shader/grass/grass.geom"
        };
        const auto gs = m_geometryShaderLoader.Load(gsSource);


        auto grassShaderProgram = std::make_shared<ShaderProgram>();
        grassShaderProgram->Build(vs, fs,&tcs, tes, gs);
        // // Terrain materials
        m_grassMaterial = std::make_unique<Material>(grassShaderProgram);
        m_grassMaterial->SetUniformValue("Color", glm::vec4(1.0f));
        m_grassMaterial->SetUniformValue("BottomColor", glm::vec4(0.06f, 0.38f, 0.07f, 1.0f));
        m_grassMaterial->SetUniformValue("TopColor", glm::vec4(0.56f, 0.83f, 0.32f, 1.0f));
        m_grassMaterial->SetUniformValue("BendRotation", 0.25f);
        m_grassMaterial->SetUniformValue("BladeWidth", 0.15f);
        m_grassMaterial->SetUniformValue("BladeWidthRandom", 0.05f);
        m_grassMaterial->SetUniformValue("BladeHeight", 2.85f);
        m_grassMaterial->SetUniformValue("BladeHeightRandom", 0.8f);
        m_grassMaterial->SetUniformValue("TessellationUniform", 8.0f);
        m_grassMaterial->SetUniformValue("WindDistortionMap", m_grassWindDistorsionMap);
        m_grassMaterial->SetUniformValue("WindFrequency", glm::vec4(0.025, 0.025, 0, 0));
        m_grassMaterial->SetUniformValue("WindStrength", 1.0f);
        m_grassMaterial->SetUniformValue("Time", 0.0f);
        m_grassMaterial->SetUniformValue("WindDistortionMapScaleOffset", glm::vec4(0.01f, 0.01f, 0.0f, 0.0f));


    }

    void InitTerrainShader() {
        {
            auto loader = Texture2DLoader(TextureObject::FormatRGB, TextureObject::InternalFormatRGB8);
            m_terrainTexture = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/dirt.png")
            );
        }

        m_terrainMesh = std::make_shared<Mesh>();
        CreateTerrainMesh(*m_terrainMesh, 512, 512, 1.0);

        const auto vs = m_vertexShaderLoader.Load("shaders/grass_geometry_shader/terrain/terrain.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_geometry_shader/terrain/terrain.frag");

        auto shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->Build(vs, fs);

        // // Terrain materials
        m_terrainMaterial = std::make_unique<Material>(shaderProgram);
        m_terrainMaterial->SetUniformValue("Color", glm::vec4(1.0f));
        m_terrainMaterial->SetUniformValue("ColorTextureScale", glm::vec2(0.125f));
        m_terrainMaterial->SetUniformValue("AlbedoTexture", m_terrainTexture);
    }

public:

    void Initialize(const Window& window) override {
        InitGrassShader();
        InitTerrainShader();

        m_camera.Initialize(window);
    }

    void Update(const Window& window, const float deltaTime) override {
        m_camera.UpdateCamera(window, deltaTime);

        m_grassMaterial->SetUniformValue("Time", m_application.GetCurrentTime());
    }

    void Render() override {
        const auto& worldMatrix = glm::mat4(1.0f);

        Render(*m_terrainMaterial, *m_terrainMesh, worldMatrix);
        Render(*m_grassMaterial, *m_grassMesh, worldMatrix);
    }

private:

    void Render(const Material& material, const Mesh& mesh, const glm::mat4& worldMatrix) const {
        const auto& shaderProgram = *material.GetShaderProgram();

        material.Use();

        const auto locationWorldMatrix = shaderProgram.GetUniformLocation("WorldMatrix");
        const auto locationViewProjMatrix = shaderProgram.GetUniformLocation("ViewProjMatrix");

        material.GetShaderProgram()->SetUniform(locationWorldMatrix, worldMatrix);
        material.GetShaderProgram()->SetUniform(locationViewProjMatrix, m_camera.GetViewProjectionMatrix());

        mesh.DrawSubmesh(0);
    }

};
