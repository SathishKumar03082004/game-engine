#pragma once

#include "../Camera/Camera2D.h"
#include "../Scene/Scene.h"

#include "../../Editor/Hierarchy/Hierarchy.h"
#include "../../Editor/Inspector/Inspector.h"
#include "../../Editor/Gizmos/TransformGizmo.h"
#include "../../Editor/History/EditorHistory.h"
#include "../Resources/ResourceManager.h"
#include "../../Editor/Inspector/RenderSettingsPanel.h"
#include "../../Editor/Assets/AssetBrowser.h"
#include "../Project/ProjectSettings.h"
#include "../Project/SceneManager.h"
#include "../../Editor/Inspector/ProjectSettingsPanel.h"
#include "../../Editor/Inspector/DebugOverlay.h"
#include "../../Editor/Tilemap/TilemapEditor.h"
#include <vector>

class Engine
{
public:
    Engine();

    void Initialize();
    void Run();
    void Shutdown();

private:
    struct TransformSnapshot
    {
        GameObject* object = nullptr;
        TransformState transform{};
    };

    TransformState CaptureTransform(GameObject* object);

    Camera2DSystem camera;
    Scene scene;

    EditorHistory history;

    Hierarchy* hierarchy;
    Inspector* inspector;
    RenderSettingsPanel* renderSettingsPanel;
    AssetBrowser* assetBrowser;
    ProjectSettings projectSettings;
    SceneManager sceneManager;
    ProjectSettingsPanel* projectSettingsPanel;
    TilemapEditor* tilemapEditor;
    DebugOverlay debugOverlay;

    TransformGizmo transformGizmo;

    GameObject* sceneDragObject;
    std::vector<TransformSnapshot> sceneDragTransforms;
    bool sceneDragging;

    GameObject* gizmoDragObject;
    std::vector<TransformSnapshot> gizmoDragTransforms;
    bool gizmoWasDragging;
};
