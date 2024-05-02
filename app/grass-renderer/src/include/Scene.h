#pragma once

#include "itugl/application/Window.h"

class Scene {
public:
    virtual ~Scene() = default;

    virtual void Initialize(const Window& window) = 0;

    virtual void Update(const Window& window, float deltaTime) = 0;

    virtual void Render() = 0;

};
