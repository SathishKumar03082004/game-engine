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

private:
    ProjectSettings& settings;
    SceneManager& scenes;
    Scene& scene;
    EditorHistory& history;
    Rectangle panel;
    std::string status;
};
