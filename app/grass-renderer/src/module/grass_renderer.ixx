module;

#include <_pch.h>

#include "itugl/application/Application.h"
#include "itugl/asset/ShaderLoader.h"
#include "itugl/camera/Camera.h"
#include "itugl/geometry/Mesh.h"
#include "itugl/shader/Material.h"

export module app;

import terrain.heightmap_cpu;
import terrain.heightmap_gpu;

import app.camera;
import app.ui;

export class GrassRenderer final : public Application {

    UIManager m_ui;

    TerrainHeightmapCPU m_terrainHeightmapCPU;
    TerrainHeightmapGPU m_terrainHeightmapGPU;

protected:

    void Initialize() override {

        auto& window = GetMainWindow();

        m_ui.Initialize(window);

        m_terrainHeightmapCPU.Initialize(window);
        m_terrainHeightmapGPU.Initialize(window);

        GetDevice().EnableFeature(GL_DEPTH_TEST); // Enable depth test
        GetDevice().SetWireframeEnabled(true); // Enable wireframe
    }

    void Update() override {
        Application::Update();

        const Window& window = GetMainWindow();
        const auto deltaTime = GetDeltaTime();

        m_terrainHeightmapGPU.Update(window, deltaTime);

        m_ui.Update();
    }

    void Render() override {
        Application::Render();

        // Clear color and depth
        GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

        m_terrainHeightmapGPU.Render();

        m_ui.Render();
    }

    void Cleanup() override {
        Application::Cleanup();

        m_ui.Clean();
    }

public:
    GrassRenderer(): Application(2560, 1440, "Grass Renderer") {}
};
