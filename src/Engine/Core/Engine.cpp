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

// Load textures after the window and graphics context are initialized.
auto& objects = scene.GetGameObjects();

for (const auto& object : objects)
{
    if (!object)
    {
        continue;
    }

    if (object->GetName() == "Player")
    {
        object->SetSprite("resources/sprites/player.png");
    }
    else if (object->GetName() == "Enemy")
    {
        object->SetSprite("resources/sprites/enemy.png");
    }
    else if (object->GetName() == "Platform")
    {
        object->SetSprite("resources/sprites/platform.png");
    }
}

    hierarchy = new Hierarchy(scene, history);
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

        if (hierarchy != nullptr)
        {
            hierarchy->Update();
        }

        if (inspector != nullptr)
        {
            inspector->Update();
        }

        Vector2 mouseScreen = GetMousePosition();
        Camera2D currentCamera = camera.GetCamera();

        Vector2 mouseWorld = GetScreenToWorld2D(
            mouseScreen,
            currentCamera
        );

        bool overHierarchy =
            hierarchy != nullptr && hierarchy->IsMouseOver();

        bool overInspector =
            inspector != nullptr && inspector->IsMouseOver();

        bool allowInteraction =
            !overHierarchy && !overInspector;

        bool wasGizmoDragging = transformGizmo.IsDragging();

        bool gizmoHandled = transformGizmo.Update(
            scene,
            mouseWorld,
            currentCamera.zoom,
            allowInteraction
        );

        bool isGizmoDragging = transformGizmo.IsDragging();

        // Start recording a gizmo transform.
        if (!wasGizmoDragging && isGizmoDragging)
        {
            gizmoDragObject = scene.GetSelectedObject();
            gizmoDragStart = CaptureTransform(gizmoDragObject);
            gizmoWasDragging = true;
        }

        // Record the completed gizmo transform.
        if (wasGizmoDragging && !isGizmoDragging)
        {
            if (gizmoWasDragging && gizmoDragObject != nullptr)
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

        // Select an object and remember its initial transform.
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            allowInteraction &&
            !gizmoHandled)
        {
            scene.SelectObject(mouseWorld);

            sceneDragObject = scene.GetSelectedObject();
            sceneDragStart = CaptureTransform(sceneDragObject);

            sceneDragging = sceneDragObject != nullptr;
        }

        // Drag the selected object.
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
            allowInteraction &&
            !gizmoHandled)
        {
            scene.DragSelectedObject(mouseWorld);
        }

        // Record the completed object drag.
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

        // Undo and redo.
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

        // Render.
        BeginDrawing();
        ClearBackground(RAYWHITE);

        camera.Begin();

        scene.Draw();

        transformGizmo.Draw(
            scene,
            currentCamera.zoom
        );

        camera.End();

        if (hierarchy != nullptr)
        {
            hierarchy->Draw();
        }

        if (inspector != nullptr)
        {
            inspector->Draw();
        }

        EndDrawing();
    }
}

void Engine::Shutdown()
{
    delete inspector;
    inspector = nullptr;

    delete hierarchy;
    hierarchy = nullptr;

    CloseWindow();
}