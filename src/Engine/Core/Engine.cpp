#include "Engine.h"
#include <raylib.h>


void DrawGrid2D(int screenWidth, int screenHeight, int gridSize){
    for(int x=0; x<=screenWidth; x+=gridSize){
        DrawLine(x, 0, x, screenHeight, LIGHTGRAY);
    }

    for(int y=0; y<=screenHeight; y+=gridSize){
        DrawLine(0, y, screenWidth, y, LIGHTGRAY);
    }
}


void Engine::Initialize()
{
    InitWindow(1280, 720, "My 2D Engine");

    SetTargetFPS(60);
}

void Engine::Run()
{
    while (!WindowShouldClose())
    {
        int width = GetScreenWidth();
        int height = GetScreenHeight();

        BeginDrawing();

        ClearBackground(RAYWHITE);

        // DrawText("My 2D Engine",300,350,30,BLACK);
        DrawGrid2D(width, height, 64);

        EndDrawing();
    }
}

void Engine::Shutdown()
{
    CloseWindow();
}