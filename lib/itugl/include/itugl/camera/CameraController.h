#pragma once

#include <glm/vec2.hpp>
#include <memory>

class SceneCamera;
class Window;
class DearImGui;

class CameraController
{
public:
    CameraController();

    [[nodiscard]] bool IsEnabled() const { return m_enabled; }
    void SetEnabled(const bool enabled) { m_enabled = enabled; }

    std::shared_ptr<SceneCamera> GetCamera() { return m_camera; }
    [[nodiscard]] std::shared_ptr<const SceneCamera> GetCamera() const { return m_camera; }
    void SetCamera(std::shared_ptr<SceneCamera> camera) { m_camera = camera; }

    void SetTranslationSpeed(const float speed) { m_translationSpeed = speed; }
    void SetRotationSpeed(const float speed) { m_rotationSpeed = speed; }

    void Update(const Window& window, float deltaTime);
    void DrawGUI(DearImGui& imGui);

private:
    void UpdateEnabled(const Window& window);
    void UpdateTranslation(const Window& window, float deltaTime);
    void UpdateRotation(const Window& window, float deltaTime);

private:
    bool m_enabled;
    bool m_enablePressed;
    std::shared_ptr<SceneCamera> m_camera;
    glm::vec2 m_mousePosition;
    float m_translationSpeed;
    float m_rotationSpeed;
};
