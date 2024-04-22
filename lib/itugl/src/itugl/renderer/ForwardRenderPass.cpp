#include <itugl/renderer/ForwardRenderPass.h>

#include <itugl/camera/Camera.h>
#include <itugl/shader/Material.h>
#include <itugl/geometry/VertexArrayObject.h>
#include <itugl/renderer/Renderer.h>

ForwardRenderPass::ForwardRenderPass()
    : ForwardRenderPass(0)
{
}

ForwardRenderPass::ForwardRenderPass(int drawcallCollectionIndex)
    : m_drawcallCollectionIndex(drawcallCollectionIndex)
{
}

void ForwardRenderPass::Render()
{
    Renderer& renderer = GetRenderer();

    const Camera& camera = renderer.GetCurrentCamera();
    const auto& lights = renderer.GetLights();
    const auto& drawcallCollection = renderer.GetDrawcalls(m_drawcallCollectionIndex);

    // for all drawcalls
    for (const Renderer::DrawcallInfo& drawcallInfo : drawcallCollection)
    {
        // Prepare drawcall states
        renderer.PrepareDrawcall(drawcallInfo);

        std::shared_ptr<const ShaderProgram> shaderProgram = drawcallInfo.material.GetShaderProgram();

        //for all lights
        bool first = true;
        unsigned int lightIndex = 0;
        while (renderer.UpdateLights(shaderProgram, lights, lightIndex))
        {
            // Set the renderstates
            renderer.SetLightingRenderStates(first);

            // Draw
            drawcallInfo.drawcall.Draw();

            first = false;
        }
    }
}
