module;

#include <_pch.h>

#include "itugl/application/Application.h"
#include "itugl/asset/ShaderLoader.h"
#include "itugl/camera/CameraController.h"
#include "itugl/camera/Camera.h"
#include "itugl/lighting/DirectionalLight.h"
#include "itugl/lighting/PointLight.h"
#include "itugl/renderer/Renderer.h"
#include "itugl/scene/ImGuiSceneVisitor.h"
#include "itugl/scene/RendererSceneVisitor.h"
#include "itugl/scene/Scene.h"
#include "itugl/scene/SceneCamera.h"
#include "itugl/scene/SceneLight.h"
#include "itugl/shader/Material.h"
#include "itugl/utils/DearImGui.h"

export module app.grass_renderer_common;

export class GrassRenderer : public Application {

protected:

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;
    ShaderLoader m_computeShaderLoader;
    ShaderLoader m_tassellationControlShaderLoader;
    ShaderLoader m_tassellationEvaluationShaderLoader;
    ShaderLoader m_geometryShaderLoader;

    Renderer m_renderer;
    DearImGui m_imGui;
    CameraController m_cameraController;
    Scene m_scene;

    std::shared_ptr<Camera> m_camera;

    void InitCallback() { // NOLINT(*-convert-member-functions-to-static)
        auto& window = GetMainWindow();

        static bool wireframeEnabled = false;
        static bool vsyncEnabled = true;

        glfwSetWindowUserPointer(window.GetInternalWindow(), &GetDevice());
        glfwSetKeyCallback(
            window.GetInternalWindow(),
            [] (
                GLFWwindow * _window,
                const int key,
                int scancode,
                const int action,
                int mods
            ) {
                const auto device = static_cast<DeviceGL*>(glfwGetWindowUserPointer(_window));

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

    void InitRender() { // NOLINT(*-convert-member-functions-to-static)
        auto& device = GetDevice();

        // TODO enable SRGB and CULL_FACE
        // device.EnableFeature(GL_FRAMEBUFFER_SRGB);
        device.EnableFeature(GL_DEPTH_TEST);
        // device.EnableFeature(GL_CULL_FACE);
        device.EnableFeature(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        device.SetVSyncEnabled(true);
    }

    void InitLights() { // NOLINT(*-convert-member-functions-to-static)
        // Create a directional light and add it to the scene
        auto directionalLight = std::make_shared<DirectionalLight>();
        directionalLight->SetPosition(glm::vec3(-20, 70, 20));
        directionalLight->SetDirection(glm::vec3(-0.3f, -1.0f, -0.3f)); // It will be normalized inside the function
        directionalLight->SetIntensity(0.2f);
        m_scene.AddSceneNode(std::make_shared<SceneLight>("directional light", directionalLight));

        // Create a point light and add it to the scene
        // auto pointLight = std::make_shared<PointLight>();
        // pointLight->SetPosition(glm::vec3(0, 0, 0));
        // pointLight->SetDistanceAttenuation(glm::vec2(5.0f, 10.0f));
        // m_scene.AddSceneNode(std::make_shared<SceneLight>("point light", pointLight));
    }

    void InitCamera(
        const glm::vec3& position,
        const glm::vec3& lookAt,
        const glm::vec3& up
    ) { // NOLINT(*-convert-member-functions-to-static)
        const auto& window = GetMainWindow();

        // Create the main camera
        m_camera = std::make_shared<Camera>();
        m_camera->SetViewMatrix(position, lookAt, up);
        const auto aspectRatio = window.GetAspectRatio();
        m_camera->SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 3000.0f);

        // Create a scene node for the camera
        const auto sceneCamera = std::make_shared<SceneCamera>("camera", m_camera);

        // Add the camera node to the scene
        m_scene.AddSceneNode(sceneCamera);

        // Set the camera scene node to be controlled by the camera controller
        m_cameraController.SetCamera(sceneCamera);
        m_cameraController.SetTranslationSpeed(20.0f);
        m_cameraController.SetRotationSpeed(0.5f);
    }

    void InitGUI() {
        auto& window = GetMainWindow();
        m_imGui.Initialize(window);
    }

    void Initialize() override {
        InitRender();
        InitCallback();
        InitGUI();
    }

    void Update() override
    {
        Application::Update();

        // Update camera controller
        m_cameraController.Update(GetMainWindow(), GetDeltaTime());

        // Add the scene nodes to the renderer
        RendererSceneVisitor rendererSceneVisitor(m_renderer);
        m_scene.AcceptVisitor(rendererSceneVisitor);
    }

    void Render() override
    {
        Application::Render();

        GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

        // Render the scene
        m_renderer.Render();

        // Render the debug user interface
        RenderGUI();
    }

    virtual void RenderGUI() {
        m_imGui.BeginFrame();

        // Draw GUI for scene nodes, using the visitor pattern
        ImGuiSceneVisitor imGuiVisitor(m_imGui, "Scene");
        m_scene.AcceptVisitor(imGuiVisitor);

        // Draw GUI for camera controller
        m_cameraController.DrawGUI(m_imGui);


        m_imGui.EndFrame();
    }

    void Cleanup() override {
        m_imGui.Cleanup();

        Application::Cleanup();
    }

    void AddStandardLightUniform(const std::shared_ptr<ShaderProgram>& shaderProgram) {

        const auto cameraPositionLocation = shaderProgram->GetUniformLocation("CameraPosition");
        const auto worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
        const auto viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");
        const auto timeLocation = shaderProgram->GetUniformLocation("Time");

        m_renderer.RegisterShaderProgram(shaderProgram,
            [=](const ShaderProgram& _shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
            {
                if (cameraChanged)
                {
                    _shaderProgram.SetUniform(cameraPositionLocation, camera.ExtractTranslation());
                    _shaderProgram.SetUniform(viewProjMatrixLocation, camera.GetViewProjectionMatrix());
                }
                _shaderProgram.SetUniform(worldMatrixLocation, worldMatrix);
                _shaderProgram.SetUniform(timeLocation, GetCurrentTime());
            },
            m_renderer.GetDefaultUpdateLightsFunction(*shaderProgram)
        );

        ShaderUniformCollection::NameSet filteredUniforms;

        filteredUniforms.insert("CameraPosition");
        filteredUniforms.insert("WorldMatrix");
        filteredUniforms.insert("ViewProjMatrix");
        filteredUniforms.insert("Time");
        filteredUniforms.insert("LightIndirect");
        filteredUniforms.insert("LightColor");
        filteredUniforms.insert("LightPosition");
        filteredUniforms.insert("LightDirection");
        filteredUniforms.insert("LightAttenuation");

        assert(shaderProgram);
    }

    void AddMVCUniform(const std::shared_ptr<ShaderProgram>& shaderProgram) {

        const auto worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
        const auto viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");

        m_renderer.RegisterShaderProgram(shaderProgram,
            [=](const ShaderProgram& _shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
            {
                if (cameraChanged)
                {
                    _shaderProgram.SetUniform(viewProjMatrixLocation, camera.GetViewProjectionMatrix());
                }
                _shaderProgram.SetUniform(worldMatrixLocation, worldMatrix);
            },
            m_renderer.GetDefaultUpdateLightsFunction(*shaderProgram)
        );

        ShaderUniformCollection::NameSet filteredUniforms;

        filteredUniforms.insert("WorldMatrix");
        filteredUniforms.insert("ViewProjMatrix");

        assert(shaderProgram);
    }

public:
    GrassRenderer(): Application(2560, 1440, "Grass Renderer"),
        m_vertexShaderLoader(Shader::VertexShader),
        m_fragmentShaderLoader(Shader::FragmentShader),
        m_computeShaderLoader(Shader::ComputeShader),
        m_tassellationControlShaderLoader(Shader::TesselationControlShader),
        m_tassellationEvaluationShaderLoader(Shader::TesselationEvaluationShader),
        m_geometryShaderLoader(Shader::GeometryShader),
        m_renderer(GetDevice()) {}
};
