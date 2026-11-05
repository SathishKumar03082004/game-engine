#pragma once
#include "raylib.h"
#include "../../Engine/Project/ProjectSettings.h"
#include <string>

class Scene;
class SceneManager;
class BuildSettingsPanel
{
public:
    BuildSettingsPanel(ProjectSettings& settings, SceneManager& scenes, Scene& scene);
    void SetOpen(bool value) { open = value; }
    bool IsOpen() const { return open; }
    void Update();
    void Draw() const;
    bool IsMouseOver() const;
private:
    ProjectSettings& settings;
    SceneManager& scenes;
    Scene& scene;
    Rectangle panel;
    bool open = false;
    std::string status;
};
