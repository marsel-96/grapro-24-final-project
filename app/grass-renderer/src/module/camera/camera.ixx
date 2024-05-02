module;

#include <_pch.h>

#include "itugl/application/Window.h"
#include "ituGL/camera/Camera.h"

export module app.camera;

export class ManagedCamera {

    Camera m_camera;

    bool m_cameraEnabled = false;
    bool m_cameraEnablePressed = false;

    float m_cameraTranslationSpeed;
    float m_cameraRotationSpeed;

    glm::vec3 m_cameraPosition;
    glm::vec3 m_cameraLookAt;

    glm::vec2 m_mousePosition {};

public:

    ManagedCamera(
        const float m_camera_translation_speed,
        const float m_camera_rotation_speed,
        const glm::vec3 &m_camera_position,
        const glm::vec3 &m_camera_look_at)
        : m_cameraTranslationSpeed(m_camera_translation_speed),
          m_cameraRotationSpeed(m_camera_rotation_speed),
          m_cameraPosition(m_camera_position),
          m_cameraLookAt(m_camera_look_at) {
    }

    void Initialize(const Window& window) {
        // Set view matrix, from the camera position looking to the origin
        m_camera.SetViewMatrix(m_cameraPosition, m_cameraLookAt);

        // Set perspective matrix
        const auto aspectRatio = window.GetAspectRatio();
        m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 3000.0f);
    }

    void UpdateCamera(const Window& window, const float deltaTime) {
        // Update if camera is enabled (controlled by SPACE key)
        {
            bool enablePressed = window.IsKeyPressed(GLFW_KEY_SPACE);
            if (enablePressed && !m_cameraEnablePressed) {
                m_cameraEnabled = !m_cameraEnabled;

                window.SetMouseVisible(!m_cameraEnabled);
                m_mousePosition = window.GetMousePosition(true);
            }
            m_cameraEnablePressed = enablePressed;
        }

        if (!m_cameraEnabled) {
            return;
        }


        glm::mat4 viewTransposedMatrix = transpose(m_camera.GetViewMatrix());
        glm::vec3 viewRight = viewTransposedMatrix[0];
        glm::vec3 viewForward = -viewTransposedMatrix[2];

        // Update camera translation
        {
            glm::vec2 inputTranslation(0.0f);

            if (window.IsKeyPressed(GLFW_KEY_A))
                inputTranslation.x = -1.0f;
            else if (window.IsKeyPressed(GLFW_KEY_D))
                inputTranslation.x = 1.0f;

            if (window.IsKeyPressed(GLFW_KEY_W))
                inputTranslation.y = 1.0f;
            else if (window.IsKeyPressed(GLFW_KEY_S))
                inputTranslation.y = -1.0f;

            inputTranslation *= m_cameraTranslationSpeed;
            inputTranslation *= deltaTime;

            // Double speed if SHIFT is pressed
            if (window.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
                inputTranslation *= 4.0f;

            m_cameraPosition += inputTranslation.x * viewRight + inputTranslation.y * viewForward;
        }

        // Update camera rotation
        {
            glm::vec2 mousePosition = window.GetMousePosition(true);
            glm::vec2 deltaMousePosition = mousePosition - m_mousePosition;
            m_mousePosition = mousePosition;

            glm::vec3 inputRotation(-deltaMousePosition.x, deltaMousePosition.y, 0.0f);

            inputRotation *= m_cameraRotationSpeed;

            viewForward = glm::rotate(inputRotation.x, glm::vec3(0, 1, 0)) * glm::rotate(
                              inputRotation.y, glm::vec3(viewRight)) * glm::vec4(viewForward, 0);
        }

        // Update view matrix
        m_camera.SetViewMatrix(m_cameraPosition, m_cameraPosition + viewForward);
    }

    [[nodiscard]] glm::mat4 GetViewProjectionMatrix() const {
        return m_camera.GetViewProjectionMatrix();
    }

    [[nodiscard]] glm::mat4 GetViewMatrix() const {
        return m_camera.GetViewMatrix();
    }

};
