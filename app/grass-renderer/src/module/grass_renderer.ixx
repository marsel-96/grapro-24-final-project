module;

#include <_pch.h>

#include "Scene.h"

#include "itugl/application/Application.h"
#include "itugl/asset/ShaderLoader.h"
#include "itugl/geometry/Mesh.h"
#include "itugl/shader/Material.h"

export module app;

import terrain.heightmap_cpu;
import terrain.heightmap_gpu;
import terrain.grass_geromertry_shader;

import app.camera;
import app.ui;

export class GrassRenderer final : public Application {

    UIManager m_ui;

    std::vector<std::unique_ptr<Scene>> m_scenes;
    unsigned short m_currentSceneIndex = 1;

    Scene* m_currentscene = nullptr;

protected:

    void Initialize() override {

        auto& window = GetMainWindow();

        m_ui.Initialize(window);

        m_scenes.emplace_back(std::make_unique<TerrainHeightmapCPU>());
        m_scenes.emplace_back(std::make_unique<TerrainHeightmapGPU>());

        for (const auto& scene : m_scenes) {
            scene->Initialize(window);
        }

        GetDevice().EnableFeature(GL_DEPTH_TEST); // Enable depth test
        GetDevice().SetWireframeEnabled(true); // Enable wireframe

        if (m_currentSceneIndex >= 0) {
            m_currentscene = m_scenes[m_currentSceneIndex].get();
        }
    }

    void Update() override {
        Application::Update();

        const Window& window = GetMainWindow();
        const auto deltaTime = GetDeltaTime();

        m_currentscene->Update(window, deltaTime);

        m_ui.Update();
    }

    void Render() override {
        Application::Render();

        // Clear color and depth
        GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

        m_currentscene->Render();

        m_ui.Render();
    }

    void Cleanup() override {
        Application::Cleanup();

        m_ui.Clean();
    }

public:
    GrassRenderer(): Application(2560, 1440, "Grass Renderer") {}
};
