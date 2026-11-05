#pragma once
#include "../../Engine/Scene/Scene.h"

class UIEditor
{
public:
    explicit UIEditor(Scene& sceneReference) : scene(sceneReference) {}
    void Update();
    void Draw() const;
    bool IsMouseOver() const;
private:
    Scene& scene;
    bool visible = false;
    GameObject* EnsureCanvas();
    void AddControl(int type);
};
