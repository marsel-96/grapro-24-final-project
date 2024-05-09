module;

#include <memory>
#include <_pch.h>

#include "Scene.h"
#include "itugl/application/Application.h"
#include "itugl/application/Window.h"
#include "ituGL/shader/ShaderUniformCollection.h"

#include "ituGL/core/BufferObject.h"
#include "ituGL/asset/ShaderLoader.h"
#include "itugl/asset/Texture2DLoader.h"
#include "itugl/buffer/SharedStorageBufferObject.h"
#include "itugl/buffer/UniformBufferObject.h"
#include "itugl/compute/Compute.h"
#include "itugl/geometry/Mesh.h"
#include "ituGL/geometry/VertexFormat.h"
#include "ituGL/shader/Material.h"

export module terrain.grass_compute_shader;

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

GrassMeshVertex GenerateGrassVertex(
    const glm::vec3 &position,
    const glm::vec3 &normal,
    const glm::vec3 &tangent,
    const glm::vec2 &uv
) {
    return GrassMeshVertex(position, normal, tangent, uv);
}

void CreateGrassMesh(
    Mesh &mesh,
    const int width,
    const int height,
    const unsigned int bladeSegments
) {
    // Define the vertex format (should match the vertex structure)
    VertexFormat vertexFormat;

    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(2);

    std::vector<GrassMeshVertex> vertices;

    constexpr auto normal = glm::vec3(0, 0, 1);
    constexpr auto tangent = glm::vec3(0, 1, 0);

    const auto width_half = static_cast<float>(width) / 2.0f;

    vertices.emplace_back(GenerateGrassVertex(glm::vec3(-width_half, 0, 0), normal, tangent, glm::vec2(0, 0)));
    vertices.emplace_back(GenerateGrassVertex(glm::vec3(width_half, 0, 0), normal, tangent, glm::vec2(1, 0)));

    for (int i = 1; i < bladeSegments; i++)
    {
        const auto t = i / static_cast<float>(bladeSegments);

        // Add below the line declaring float t.
        const float segmentHeight = height * t;
        const float segmentWidth = width_half * (1 - t);

        vertices.emplace_back(GenerateGrassVertex(glm::vec3(-segmentWidth, segmentHeight, 0), normal, tangent, glm::vec2(0, t)));
        vertices.emplace_back(GenerateGrassVertex(glm::vec3(+segmentWidth, segmentHeight, 0), normal, tangent, glm::vec2(1, t)));
    }

    vertices.emplace_back(GenerateGrassVertex(glm::vec3(0, height,0), normal, tangent, glm::vec2(0.5, 1)));

    mesh.AddSubmesh<GrassMeshVertex, VertexFormat::LayoutIterator>(
        Drawcall::Primitive::TriangleStrip,
        vertices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true),
        vertexFormat.LayoutEnd());

}

using ComputeVertices = glm::vec4;
std::pair<std::vector<ComputeVertices>, std::vector<unsigned int>> GetTerrainMeshPoints(
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

    std::vector<ComputeVertices> vertices;
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
            glm::vec4 position(
                -f_width / 2.0f + f_width * f_i / f_res,
                1.0f,
                -f_height / 2.0f + f_height * f_j / f_res,
                1.0f
            );

            vertices.emplace_back(position);

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

    return std::pair(vertices, indices);
}

export class GrassComputeShader final : public Scene {

    const Application& m_application;

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;
    ShaderLoader m_tassellationControlShaderLoader;
    ShaderLoader m_tassellationEvaluationShaderLoader;
    ShaderLoader m_geometryShaderLoader;
    ShaderLoader m_computeShaderLoader;

    ManagedCamera m_camera;

    std::shared_ptr<Material> m_terrainMaterial;
    std::shared_ptr<Material> m_grassMaterial;

    std::shared_ptr<Texture2DObject> m_grassWindDistorsionMap;
    std::shared_ptr<Texture2DObject> m_terrainTexture;

    std::shared_ptr<Mesh> m_grassMesh;
    std::shared_ptr<Mesh> m_terrainMesh;

    unsigned int m_patchWidth = 512;
    unsigned int m_patchHeight = 512;

    unsigned int m_trianglesCount = 0;

    std::shared_ptr<ComputeCall> m_grassCompute;

public:
    explicit GrassComputeShader(const Application& application)
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

    void InitGrassCompute() {
        const auto cs = m_computeShaderLoader.Load("shaders/grass_compute_shader/grass/grass.comp");
        auto shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->Build(cs);

        const auto &[vertices, indices] = GetTerrainMeshPoints(m_patchWidth, m_patchHeight, m_patchWidth);

        const auto ssboPositions = std::make_shared<SharedStorageBufferObject>();
        ssboPositions->Bind();
        ssboPositions->AllocateData<ComputeVertices>(vertices);
        ssboPositions->Unbind();

        const auto ssboIndices = std::make_shared<SharedStorageBufferObject>();
        ssboIndices->Bind();
        ssboIndices->AllocateData<unsigned int>(indices);
        ssboIndices->Unbind();

        const auto ssboMatrices = std::make_shared<SharedStorageBufferObject>();

        ssboMatrices->Bind();
        ssboMatrices->AllocateData<glm::mat4>(m_patchWidth * m_patchHeight);
        ssboMatrices->Unbind();

        m_trianglesCount = m_patchWidth * m_patchHeight * 2;
        m_grassCompute = std::make_shared<ComputeCall>(shaderProgram, glm::uvec3(m_trianglesCount / 64, 1, 1));

        m_grassCompute->SetUniformValue("TerrainTriangleCount", m_trianglesCount);
        m_grassCompute->SetUniformValue("Scale", 1.0f);
        m_grassCompute->SetUniformValue("MinBladeHeight", 1.0f);
        m_grassCompute->SetUniformValue("MaxBladeHeight", 1.5f);
        m_grassCompute->SetUniformValue("MinOffset", 0.0f);
        m_grassCompute->SetUniformValue("MaxOffset", 0.25f);
        m_grassCompute->SetUniformValue("TerrainObjectToWorld", glm::mat4(1.0));

        m_grassCompute->AddBufferBinding(ssboMatrices, 0);
        m_grassCompute->AddBufferBinding(ssboPositions, 1);
        m_grassCompute->AddBufferBinding(ssboIndices, 2);

    }

    void InitGrassShader() {
        {
            auto loader = Texture2DLoader(TextureObject::FormatRG, TextureObject::InternalFormatRG8);
            m_grassWindDistorsionMap = std::make_unique<Texture2DObject>(
                loader.Load("assets/textures/wind.png")
            );
        }

        m_grassMesh = std::make_shared<Mesh>();
        CreateGrassMesh(*m_grassMesh, 1, 4, 5);

        const auto vs = m_vertexShaderLoader.Load("shaders/grass_compute_shader/grass/grass.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_compute_shader/grass/grass.frag");

        auto grassShaderProgram = std::make_shared<ShaderProgram>();
        grassShaderProgram->Build(vs, fs);

        // // Terrain materials
        m_grassMaterial = std::make_unique<Material>(grassShaderProgram);
        m_grassMaterial->SetUniformValue("Color", glm::vec4(1.0f));
        m_grassMaterial->SetUniformValue("BottomColor", glm::vec4(0.06f, 0.38f, 0.07f, 1.0f));
        m_grassMaterial->SetUniformValue("TopColor", glm::vec4(0.56f, 0.83f, 0.32f, 1.0f));

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

        const auto vs = m_vertexShaderLoader.Load("shaders/grass_compute_shader/terrain/terrain.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/grass_compute_shader/terrain/terrain.frag");

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
        InitGrassCompute();

        Compute();

        m_camera.Initialize(window);
    }

    void Update(const Window& window, const float deltaTime) override {
        m_camera.UpdateCamera(window, deltaTime);

        m_grassMaterial->SetUniformValue("Time", m_application.GetCurrentTime());
    }

    void Render() override {
        const auto& worldMatrix = glm::mat4(1.0f);

        m_grassCompute->BindBuffers();

        Render(*m_terrainMaterial, *m_terrainMesh, worldMatrix);
        Render(*m_grassMaterial, *m_grassMesh, worldMatrix, m_trianglesCount);
    }

    void Compute() const {
        m_grassCompute->Compute();

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

private:

    void Render(const Material& material, const Mesh& mesh, const glm::mat4& worldMatrix, const unsigned int instances = 1) const {
        const auto& shaderProgram = *material.GetShaderProgram();

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
