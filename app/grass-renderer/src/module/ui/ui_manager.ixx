module;

#include <_pch.h>

#include "itugl/application/Window.h"
#include "itugl/utils/DearImGui.h"

export module app.ui;

export class UIManager {

    DearImGui m_imGui;

    glm::vec4 m_ambientColor = glm::vec4(0.0f);
    glm::vec4 m_lightColor = glm::vec4(0.0f);
    float m_lightIntensity = 0.0f;
    glm::vec3 m_lightPosition = glm::vec3(0.0f, 1.0f, 0.0f);

public:
    void Initialize(Window& window) {
        m_imGui.Initialize(window);
    }

    void Update() {}

    void Render()
    {
        m_imGui.BeginFrame();

        // Add debug controls for light properties
        ImGui::ColorEdit3("Ambient color", &m_ambientColor[0]);
        ImGui::Separator();
        ImGui::DragFloat3("Light position", &m_lightPosition[0], 0.1f);
        ImGui::ColorEdit3("Light color", &m_lightColor[0]);
        ImGui::DragFloat("Light intensity", &m_lightIntensity, 0.05f, 0.0f, 100.0f);
        ImGui::Separator();

        m_imGui.EndFrame();
    }

    void Clean() {
        m_imGui.Cleanup();
    }

};
