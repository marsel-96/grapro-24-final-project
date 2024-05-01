module;

#include <_pch.h>

#include "itugl/application/Application.h"
#include "itugl/asset/ShaderLoader.h"
#include "itugl/camera/Camera.h"
#include "itugl/geometry/Mesh.h"
#include "itugl/shader/Material.h"

export module app;

import terrain.heightmap_cpu;

import app.camera;
import app.ui;

export class GrassRenderer final : public Application {

    ManagedCamera m_camera;
    UIManager m_ui;

    TerrainHeightmapCPU m_terrainHeightmapCPU;

protected:

    void Initialize() override {

        auto& window = GetMainWindow();

        m_ui.Initialize(window);
        m_camera.Initialize(window);
        m_terrainHeightmapCPU.Initialize();

        GetDevice().EnableFeature(GL_DEPTH_TEST); // Enable depth test
        //GetDevice().SetWireframeEnabled(true); // Enable wireframe
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
        DrawObject(
            m_terrainHeightmapCPU.GetMesh(),
            m_terrainHeightmapCPU.GetMaterial(),
            scale(glm::vec3(1.0f))
        );


        m_ui.Render();
    }

    void DrawObject(const Mesh& mesh, const Material& material, const glm::mat4& worldMatrix) const {
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
    GrassRenderer(): Application(2560, 1440, "Grass Renderer"),
                     m_camera(
                         20.0f,
                         0.5f,
                         glm::vec3(0.0f, 15.0f, 15.0f),
                         glm::vec3(0.0f, 0.0f, 0.0f)
                     ) {
    }
};
