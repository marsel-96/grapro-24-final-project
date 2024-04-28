#include "RaymarchingApplication.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <itugl/asset/ShaderLoader.h>
#include <itugl/camera/Camera.h>
#include <itugl/scene/SceneCamera.h>
#include <itugl/lighting/DirectionalLight.h>
#include <itugl/shader/Material.h>
#include <itugl/renderer/PostFXRenderPass.h>
#include <itugl/scene/RendererSceneVisitor.h>
#include <imgui.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

RaymarchingApplication::RaymarchingApplication()
    : Application(1024, 1024, "Ray-marching demo")
    , m_renderer(GetDevice())
{
}

void RaymarchingApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());

    InitializeCamera();
    InitializeMaterial();
    InitializeRenderer();
}

void RaymarchingApplication::Update()
{
    Application::Update();

    // Update camera controller
    m_cameraController.Update(GetMainWindow(), GetDeltaTime());

    // Set renderer camera
    const Camera& camera = *m_cameraController.GetCamera()->GetCamera();
    m_renderer.SetCurrentCamera(camera);

    // Update the material properties
    m_material->SetUniformValue("ProjMatrix", camera.GetProjectionMatrix());
    m_material->SetUniformValue("InvProjMatrix", glm::inverse(camera.GetProjectionMatrix()));
}

void RaymarchingApplication::Render()
{
    Application::Render();

    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Render the scene
    m_renderer.Render();

    // Render the debug user interface
    RenderGUI();
}

void RaymarchingApplication::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void RaymarchingApplication::InitializeCamera()
{
    // Create the main camera
    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
    camera->SetViewMatrix(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec3(0.0f, 1.0f, 0.0));
    float fov = 1.0f;
    camera->SetPerspectiveProjectionMatrix(fov, GetMainWindow().GetAspectRatio(), 0.1f, 100.0f);

    // Create a scene node for the camera
    std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

    // Set the camera scene node to be controlled by the camera controller
    m_cameraController.SetCamera(sceneCamera);
}

void RaymarchingApplication::InitializeMaterial()
{
    m_material = CreateRaymarchingMaterial("shaders/exercise10.glsl");

    // 10.X: Initialize material uniforms
    m_material->SetUniformValue("SphereColor", m_sphereColor);
    m_material->SetUniformValue("SphereCenter", m_sphereCenter);
    m_material->SetUniformValue("SphereRadius", m_sphereRadius);
    m_material->SetUniformValue("BoxColor", m_boxColor);
    m_material->SetUniformValue("BoxMatrix", m_boxMatrix);
    m_material->SetUniformValue("BoxSize", m_boxSize);

    m_material->SetUniformValue("Smoothness", m_unionSmoothness);
}

void RaymarchingApplication::InitializeRenderer()
{
    m_renderer.AddRenderPass(std::make_unique<PostFXRenderPass>(m_material));
}

std::shared_ptr<Material> RaymarchingApplication::CreateRaymarchingMaterial(const char* fragmentShaderPath)
{
    // We could keep this vertex shader and reuse it, but it looks simpler this way
    std::vector<const char*> vertexShaderPaths;
    vertexShaderPaths.push_back("shaders/version330.glsl");
    vertexShaderPaths.push_back("shaders/renderer/fullscreen.vert");
    Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

    std::vector<const char*> fragmentShaderPaths;
    fragmentShaderPaths.push_back("shaders/version330.glsl");
    fragmentShaderPaths.push_back("shaders/utils.glsl");
    fragmentShaderPaths.push_back("shaders/sdflibrary.glsl");
    fragmentShaderPaths.push_back("shaders/raymarcher.glsl");
    fragmentShaderPaths.push_back(fragmentShaderPath);
    fragmentShaderPaths.push_back("shaders/raymarching.frag");
    Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

    std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
    shaderProgramPtr->Build(vertexShader, fragmentShader);

    // Create material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgramPtr);

    return material;
}

void RaymarchingApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    // Draw GUI for camera controller
    //m_cameraController.DrawGUI(m_imGui);

    if (auto window = m_imGui.UseWindow("Scene parameters"))
    {
        // (todo) 10.3: Get the camera view matrix and transform the sphere center and the box matrix

        if (ImGui::TreeNodeEx("Union", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // 10.1: Add controls for sphere parameters

            if(ImGui::DragFloat("Smoothness", &m_unionSmoothness, .1f)) {
                m_material->SetUniformValue("Smoothness", m_unionSmoothness);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Sphere", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // 10.1: Add controls for sphere parameters

            if(ImGui::ColorEdit3("Sphere color", &m_sphereColor[0])) {
                m_material->SetUniformValue("SphereColor", m_sphereColor);
            }
            if(ImGui::DragFloat3("Sphere center", &m_sphereCenter[0])) {
                m_material->SetUniformValue("SphereCenter", m_sphereCenter);
            }
            if(ImGui::DragFloat("Sphere radius", &m_sphereRadius)) {
                m_material->SetUniformValue("SphereRadius", m_sphereRadius);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Box", ImGuiTreeNodeFlags_DefaultOpen))
        {

            // 10.1: Add controls for box parameters

            if(ImGui::ColorEdit3("Box color", &m_boxColor[0])) {
                m_material->SetUniformValue("BoxColor", m_boxColor);
            }
            if(ImGui::DragFloat3("Box Rotation", &m_boxMatrixRotation[0], .1f) ||
                ImGui::DragFloat3("Box Translation", &m_boxMatrixTranslation[0], .1f)) {
                const auto translation = translate(m_boxMatrixTranslation);
                const auto rotation = glm::eulerAngleYXZ(
                    m_boxMatrixRotation.x,
                    m_boxMatrixRotation.y,
                    m_boxMatrixRotation.z
                );
                m_material->SetUniformValue("BoxMatrix", translation * rotation);
            }
            if(ImGui::DragFloat3("Box size", &m_boxSize[0])) {
                m_material->SetUniformValue("BoxSize", m_boxSize);
            }

            ImGui::TreePop();
        }
    }

    m_imGui.EndFrame();
}
