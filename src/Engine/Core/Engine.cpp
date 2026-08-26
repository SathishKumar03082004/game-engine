#include "Engine.h"
#include <raylib.h>

void DrawGrid2D(int gridSize)
{
    const int gridExtent = 5000;

    for (int x = -gridExtent; x <= gridExtent; x += gridSize)
    {
        DrawLine(x,-gridExtent,x,gridExtent,LIGHTGRAY);
    }

    for (int y = -gridExtent; y <= gridExtent; y += gridSize)
    {
        DrawLine(-gridExtent,y,gridExtent,y,LIGHTGRAY);
    }
}

void Engine::Initialize()
{
    InitWindow(1280, 720, "My 2D Engine");

    SetTargetFPS(60);

    camera.Initialize();
}

void Engine::Run()
{
    while (!WindowShouldClose())
    {
        camera.Update();

        BeginDrawing();

        ClearBackground(RAYWHITE);

        camera.Begin();

        DrawGrid2D(64);

        camera.End();

        EndDrawing();
    }
}

void Engine::Shutdown()
{
    CloseWindow();
}