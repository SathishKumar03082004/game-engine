#pragma once

#include "../Camera/Camera2D.h"
#include "../Scene/Scene.h"

#include "../../Editor/Hierarchy/Hierarchy.h"
#include "../../Editor/Inspector/Inspector.h"
#include "../../Editor/Gizmos/TransformGizmo.h"
#include "../../Editor/History/EditorHistory.h"

class Engine
{
public:
    Engine();

    void Initialize();
    void Run();
    void Shutdown();

private:
    TransformState CaptureTransform(GameObject* object);

    Camera2DSystem camera;
    Scene scene;

    EditorHistory history;

    Hierarchy* hierarchy;
    Inspector* inspector;

    TransformGizmo transformGizmo;

    GameObject* sceneDragObject;
    TransformState sceneDragStart;
    bool sceneDragging;

    GameObject* gizmoDragObject;
    TransformState gizmoDragStart;
    bool gizmoWasDragging;
};