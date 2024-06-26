#pragma once

#include <itugl/scene/SceneVisitor.h>

class Renderer;
class SceneCamera;
class SceneLight;
class SceneModel;
class Transform;

class RendererSceneVisitor : public SceneVisitor
{
public:
    RendererSceneVisitor(Renderer& renderer);

    void VisitCamera(SceneCamera& sceneCamera) override;

    void VisitLight(SceneLight& sceneLight) override;

    void VisitModel(SceneModel& sceneModel) override;

private:
    Renderer& m_renderer;
};
