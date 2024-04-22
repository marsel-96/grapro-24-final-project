#pragma once

#include <itugl/application/Application.h>

#include <itugl/renderer/Renderer.h>
#include <itugl/camera/CameraController.h>
#include <itugl/utils/DearImGui.h>

class Material;

class RaymarchingApplication : public Application
{
public:
    RaymarchingApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void InitializeCamera();
    void InitializeMaterial();
    void InitializeRenderer();

    std::shared_ptr<Material> CreateRaymarchingMaterial(const char* fragmentShaderPath);

    void RenderGUI();

private:
    // Helper object for debug GUI
    DearImGui m_imGui;

    // Camera controller
    CameraController m_cameraController;

    // Renderer
    Renderer m_renderer;

    // Materials
    std::shared_ptr<Material> m_material;

    glm::vec3 m_sphereColor = glm::vec3(0, 0, 1);
    glm::vec3 m_sphereCenter = glm::vec3(-2, 0, -10);
    float m_sphereRadius = 1.25f;
    float m_unionSmoothness = 0.5f;
    glm::vec3 m_boxColor = glm::vec3(1, 0, 0);
    glm::mat4 m_boxMatrix = glm::mat4(1,0,0,0,   0,1,0,0,   0,0,1,0,   2,0,-10,1);
    glm::vec3 m_boxMatrixRotation = glm::vec3(0,0,0);
    glm::vec3 m_boxMatrixTranslation = glm::vec3(1,0,-10);
    glm::vec3 m_boxSize = glm::vec3(1, 1, 1);
};
