#pragma once
#include "../../Engine/Scene/Scene.h"
#include <string>

class UIEditor
{
public:
    explicit UIEditor(Scene& sceneReference) : scene(sceneReference) {}
    void Update();
    void Draw() const;
    bool IsMouseOver() const;
    bool IsVisible() const { return visible; }
    void SetVisible(bool value) { visible = value; }
    void CreateElement(int type);
private:
    Scene& scene;
    bool visible = false;
    std::string status;
    GameObject* EnsureCanvas();
    void AddControl(int type);
};
