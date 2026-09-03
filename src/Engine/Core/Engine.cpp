#include "Engine.h"

#include <raylib.h>

void Engine::Initialize()
{
    InitWindow(
        1280,
        720,
        "My 2D Engine"
    );

    SetTargetFPS(60);

    camera.Initialize();

    scene.Initialize();
}

void Engine::Run()
{
    while (!WindowShouldClose())
    {
        // ========================================
        // UPDATE
        // ========================================

        camera.Update();

        scene.Update();


        // ========================================
        // MOUSE
        // ========================================

        Vector2 mouseScreen = GetMousePosition();

        Vector2 mouseWorld = GetScreenToWorld2D(
            mouseScreen,
            camera.GetCamera()
        );


        // ========================================
        // SELECT
        // ========================================

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            scene.SelectObject(mouseWorld);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            scene.DragSelectedObject(mouseWorld);
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        camera.Begin();

        scene.Draw();

        camera.End();

        EndDrawing();
    }
}

void Engine::Shutdown()
{
    CloseWindow();
}