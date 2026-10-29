#include "Engine.h"
#include <raylib.h>

Engine::Engine()
    : hierarchy(nullptr),
      inspector(nullptr)
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

        bool gizmoHandled = transformGizmo.Update(
            scene,
            mouseWorld,
            currentCamera.zoom,
            allowInteraction
        );

        bool ctrlPressed =
            IsKeyDown(KEY_LEFT_CONTROL) ||
            IsKeyDown(KEY_RIGHT_CONTROL);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            allowInteraction &&
            !gizmoHandled)
        {
            scene.SelectObject(mouseWorld, ctrlPressed);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
            allowInteraction &&
            !gizmoHandled &&
            !ctrlPressed &&
            scene.GetSelectedObjects().size() == 1)
        {
            scene.DragSelectedObject(mouseWorld);
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