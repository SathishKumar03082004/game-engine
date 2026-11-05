#pragma once

#include "../../Engine/Scene/Scene.h"

class DebugOverlay
{
public:
    void Update();
    void DrawWorld(const Scene& scene) const;
    void DrawScreen(const Scene& scene, float sceneUpdateMilliseconds) const;
    bool IsVisible() const;
    void ToggleVisible() { visible = !visible; }

private:
    bool visible = false;
    bool showColliders = false;
};
