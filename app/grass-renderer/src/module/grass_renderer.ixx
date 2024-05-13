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
import terrain.grass_geometry_shader;
import terrain.grass_compute_shader;
import terrain.grass_compute_shader_2;

import app.camera;
import app.ui;

export class GrassRenderer final : public Application {

    UIManager m_ui;

    std::vector<std::unique_ptr<Scene>> m_scenes;
    unsigned short m_currentSceneIndex = 4;

    Scene* m_currentscene = nullptr;

private:

    void fixme(Window& _window) {
        static bool wireframeEnabled = false;
        static bool vsyncEnabled = true;

        glfwSetWindowUserPointer(_window.GetInternalWindow(), &GetDevice());
        glfwSetKeyCallback(
            _window.GetInternalWindow(),
            [] (
                GLFWwindow *window,
                const int key,
                int scancode,
                const int action,
                int mods
            ) {
                const auto device = static_cast<DeviceGL*>(glfwGetWindowUserPointer(window));

                if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
                {
                    wireframeEnabled = !wireframeEnabled;
                    device->SetWireframeEnabled(wireframeEnabled);
                }
                if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
                {
                    vsyncEnabled = !vsyncEnabled;
                    device->SetVSyncEnabled(vsyncEnabled);
                }
            }
        );
    }

protected:

    void Initialize() override {

        auto& window = GetMainWindow();

        m_ui.Initialize(window);

        m_scenes.emplace_back(std::make_unique<TerrainHeightmapCPU>());
        m_scenes.emplace_back(std::make_unique<TerrainHeightmapGPU>());
        m_scenes.emplace_back(std::make_unique<GrassGeometryShader>(*this));
        m_scenes.emplace_back(std::make_unique<GrassComputeShader>(*this));
        m_scenes.emplace_back(std::make_unique<GrassComputeShader2>(*this));

        GetDevice().EnableFeature(GL_DEPTH_TEST);

        fixme(window);

        m_currentscene = m_scenes[m_currentSceneIndex].get();
        m_currentscene->Initialize(window);
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
