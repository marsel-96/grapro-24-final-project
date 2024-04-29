module;

#include "../../../lib/itugl/include/itugl/application/Application.h"
#include "../../../lib/itugl/include/ituGL/asset/ShaderLoader.h"
#include "../../../lib/itugl/include/ituGL/camera/Camera.h"
#include "../../../lib/itugl/include/itugl/geometry/Mesh.h"
#include "../../../lib/itugl/include/ituGL/shader/Material.h"
#include "../../../lib/itugl/include/itugl/shader/Shader.h"

export module app;

import app.util.mesh;
import app.util.heightmap;
import app.util.texture;

import app.camera;
import app.ui;

export class GrassRenderer final : public Application {

    Mesh m_terrainMesh;
    ManagedCamera m_camera;
    UIManager m_ui;

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;

    std::unique_ptr<Material> m_defaultMaterial;
    std::unique_ptr<Material> m_terrainMaterial;

    std::shared_ptr<TextureObject> m_defaultTexture;
    std::shared_ptr<TextureObject> m_dirtTexture;

    void InitTerrain() {
        CreateTerrainMesh(m_terrainMesh, 100, 100);
    }

    void InitTextures() {
        m_defaultTexture = CreateDefaultTexture();
        m_dirtTexture = LoadTexture("assets/textures/dirt.png");
    }

    void InitDefaultShader() {
        // Default shader program
        const auto vs = m_vertexShaderLoader.Load("shaders/default.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/default.frag");

        auto defaultShaderProgram = std::make_shared<ShaderProgram>();
        defaultShaderProgram->Build(
            vs,
            fs);

        // Default material
        m_defaultMaterial = std::make_unique<Material>(defaultShaderProgram);
        m_defaultMaterial->SetUniformValue("Color", glm::vec4(1.0f));
    }

    void InitTerrainShader() {
        // Terrain shader program
        const auto vs = m_vertexShaderLoader.Load("shaders/terrain.vert");
        const auto fs = m_fragmentShaderLoader.Load("shaders/terrain.frag");

        auto terrainShaderProgram = std::make_shared<ShaderProgram>();
        terrainShaderProgram->Build(vs, fs);

        // Terrain materials
        m_terrainMaterial = std::make_unique<Material>(terrainShaderProgram);
        m_terrainMaterial->SetUniformValue("Color", glm::vec4(1.0f));
        m_terrainMaterial->SetUniformValue("AlbedoTexture", m_dirtTexture);
    }

protected:

    void Initialize() override {

        InitTerrain();
        InitTextures();
        InitDefaultShader();
        InitTerrainShader();

        auto& window = GetMainWindow();

        m_ui.Initialize(window);
        m_camera.Initialize(window);

        GetDevice().EnableFeature(GL_DEPTH_TEST); // Enable depth test
        GetDevice().SetWireframeEnabled(true); // Enable wireframe
    }

    void Update() override {
        Application::Update();

        const Window& window = GetMainWindow();

        m_ui.Update();
        m_camera.UpdateCamera(
            window,
            GetDeltaTime()
        );
    }

    void Render() override {
        Application::Render();

        // Clear color and depth
        GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

        // Terrain patches
        DrawObject(m_terrainMesh, *m_terrainMaterial, scale(glm::vec3(10.0f)));
        DrawObject(m_terrainMesh, *m_terrainMaterial, translate(glm::vec3(-10.0f, 0.0f, 0.0f)) * scale(glm::vec3(10.0f)));
        DrawObject(m_terrainMesh, *m_terrainMaterial, translate(glm::vec3(0.0f, 0.0f, -10.0f)) * scale(glm::vec3(10.0f)));
        DrawObject(m_terrainMesh, *m_terrainMaterial, translate(glm::vec3(-10.0f, 0.0f, -10.0f)) * scale(glm::vec3(10.0f)));

        m_ui.Render();
    }

    void DrawObject(const Mesh& mesh, Material& material, const glm::mat4& worldMatrix) const {
        material.Use();

        const auto& shaderProgram = *material.GetShaderProgram();

        const auto locationWorldMatrix = shaderProgram.GetUniformLocation("WorldMatrix");
        const auto locationViewProjMatrix = shaderProgram.GetUniformLocation("ViewProjMatrix");

        material.GetShaderProgram()->SetUniform(locationWorldMatrix, worldMatrix);
        material.GetShaderProgram()->SetUniform(locationViewProjMatrix, m_camera.GetViewProjectionMatrix());

        mesh.DrawSubmesh(0);
    }

    void Cleanup() override {
        Application::Cleanup();

        m_ui.Clean();
    }

public:
    GrassRenderer(): Application(1024, 768, "Grass Renderer"),
                     m_camera(
                         20.0f,
                         0.5f,
                         glm::vec3(0.0f, 15.0f, 15.0f),
                         glm::vec3(0.0f, 0.0f, 0.0f)
                     ),
                     m_vertexShaderLoader(Shader::Type::VertexShader),
                     m_fragmentShaderLoader(Shader::Type::FragmentShader) {
    }
};
