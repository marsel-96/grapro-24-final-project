#pragma once

#include <itugl/renderer/RenderPass.h>

#include <itugl/shader/ShaderProgram.h>
#include <itugl/geometry/Mesh.h>
#include <memory>

class Texture2DObject;
class Material;

class DeferredRenderPass: public RenderPass
{
public:
    DeferredRenderPass(std::shared_ptr<Material> material, std::shared_ptr<const FramebufferObject> targetFramebuffer = nullptr);

    void Render() override;

private:
    void InitializeMeshes();

private:
    std::shared_ptr<Material> m_material;
};
