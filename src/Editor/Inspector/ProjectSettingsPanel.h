#pragma once

#include "raylib.h"
#include "../../Engine/Project/ProjectSettings.h"
#include "../../Engine/Project/SceneManager.h"
#include "../../Engine/Scene/Scene.h"
#include "../History/EditorHistory.h"

#include <string>

class ProjectSettingsPanel
{
public:
    ProjectSettingsPanel(ProjectSettings& settings, SceneManager& scenes, Scene& scene, EditorHistory& history);
    void Update();
    void Draw() const;
    bool IsMouseOver() const;
    void SetOpen(bool value) { open = value; }
    bool IsOpen() const { return open; }

private:
    ProjectSettings& settings;
    SceneManager& scenes;
    Scene& scene;
    EditorHistory& history;
    Rectangle panel;
    std::string status;
    bool open = false;
};
