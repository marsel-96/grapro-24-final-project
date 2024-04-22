#include <itugl/renderer/PostFXRenderPass.h>

#include <itugl/renderer/Renderer.h>
#include <itugl/shader/Material.h>
#include <itugl/texture/FramebufferObject.h>

PostFXRenderPass::PostFXRenderPass(std::shared_ptr<Material> material, std::shared_ptr<const FramebufferObject> framebuffer)
    : RenderPass(framebuffer), m_material(material)
{
}

void PostFXRenderPass::Render()
{
    Renderer& renderer = GetRenderer();

    assert(m_material);
    m_material->Use();

    const Mesh* mesh = &renderer.GetFullscreenMesh();
    mesh->DrawSubmesh(0);
}
