#include "Engine.h"
#include <raylib.h>

Engine::Engine()
    : hierarchy(nullptr),
      inspector(nullptr),
      sceneDragObject(nullptr),
      sceneDragStart{},
      sceneDragging(false),
      gizmoDragObject(nullptr),
      gizmoDragStart{},
      gizmoWasDragging(false)
{
}

void Engine::Initialize()
{
    InitWindow(1280, 720, "My 2D Engine");
    SetTargetFPS(60);

    camera.Initialize();
    scene.Initialize();

    hierarchy = new Hierarchy(scene);
    hierarchy->Initialize();

    inspector = new Inspector(scene);
    inspector->Initialize();
}

TransformState Engine::CaptureTransform(GameObject* object)
{
    if (object == nullptr)
    {
        return {};
    }

    TransformComponent& transform = object->GetTransform();

    return {
        transform.GetPosition(),
        transform.GetRotation(),
        transform.GetScale()
    };
}

void Engine::Run()
{
    while (!WindowShouldClose())
    {
        camera.Update();

        scene.Update();
        hierarchy->Update();
        inspector->Update();

        Vector2 mouseScreen = GetMousePosition();
        Camera2D currentCamera = camera.GetCamera();

        Vector2 mouseWorld = GetScreenToWorld2D(
            mouseScreen,
            currentCamera
        );

        bool overHierarchy = hierarchy->IsMouseOver();
        bool overInspector = inspector->IsMouseOver();

        bool allowInteraction =
            !overHierarchy && !overInspector;

        bool wasGizmoDragging = transformGizmo.IsDragging();

        GameObject* selectedBeforeGizmo =
            scene.GetSelectedObject();

        TransformState gizmoStartBeforeUpdate =
            gizmoWasDragging
                ? gizmoDragStart
                : CaptureTransform(selectedBeforeGizmo);

        bool gizmoHandled = transformGizmo.Update(
            scene,
            mouseWorld,
            currentCamera.zoom,
            allowInteraction
        );

        bool isGizmoDragging = transformGizmo.IsDragging();

        if (!wasGizmoDragging && isGizmoDragging)
        {
            gizmoDragObject = scene.GetSelectedObject();
            gizmoDragStart = CaptureTransform(gizmoDragObject);
            gizmoWasDragging = true;
        }

        if (wasGizmoDragging && !isGizmoDragging)
        {
            if (gizmoDragObject != nullptr)
            {
                TransformState after =
                    CaptureTransform(gizmoDragObject);

                history.RecordTransform(
                    gizmoDragObject,
                    gizmoDragStart,
                    after
                );
            }

            gizmoDragObject = nullptr;
            gizmoWasDragging = false;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            allowInteraction &&
            !gizmoHandled)
        {
            scene.SelectObject(mouseWorld);

            sceneDragObject = scene.GetSelectedObject();
            sceneDragStart = CaptureTransform(sceneDragObject);

            sceneDragging = sceneDragObject != nullptr;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
            allowInteraction &&
            !gizmoHandled)
        {
            scene.DragSelectedObject(mouseWorld);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            if (sceneDragging && sceneDragObject != nullptr)
            {
                TransformState after =
                    CaptureTransform(sceneDragObject);

                history.RecordTransform(
                    sceneDragObject,
                    sceneDragStart,
                    after
                );
            }

            sceneDragObject = nullptr;
            sceneDragging = false;
        }

        bool ctrlDown =
            IsKeyDown(KEY_LEFT_CONTROL) ||
            IsKeyDown(KEY_RIGHT_CONTROL);

        if (ctrlDown && IsKeyPressed(KEY_Z))
        {
            history.Undo();
        }
        else if (ctrlDown && IsKeyPressed(KEY_Y))
        {
            history.Redo();
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        camera.Begin();

        scene.Draw();

        transformGizmo.Draw(
            scene,
            currentCamera.zoom
        );

        camera.End();

        hierarchy->Draw();
        inspector->Draw();

        EndDrawing();
    }
}

void Engine::Shutdown()
{
    if (inspector != nullptr)
    {
        delete inspector;
        inspector = nullptr;
    }

    if (hierarchy != nullptr)
    {
        delete hierarchy;
        hierarchy = nullptr;
    }

    CloseWindow();
}