#pragma once
#include "../Scene/Scene.h"

class UIManager
{
public:
    void Update(Scene& scene);
    void Draw(Scene& scene);
    void ReleaseResources(Scene& scene);
    GameObject* GetObjectAtPoint(Scene& scene, Vector2 screenPoint) const;
    void ResetFocus() { keyboardFocus = nullptr; }
private:
    GameObject* keyboardFocus = nullptr;
};
