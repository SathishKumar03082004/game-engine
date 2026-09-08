#include "Engine.h"

#include <raylib.h>

Engine::Engine()
{
    hierarchy = nullptr;
    inspector = nullptr;
}


void Engine::Initialize()
{
    InitWindow(1280,720,"My 2D Engine");

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

        Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen,camera.GetCamera());

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            if (!hierarchy->IsMouseOver())
            {
                scene.SelectObject(mouseWorld);
            }
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            if (!hierarchy->IsMouseOver())
            {
                scene.DragSelectedObject(mouseWorld);
            }
        }


        BeginDrawing();

        ClearBackground(RAYWHITE);


        camera.Begin();

        scene.Draw();

        camera.End();

        hierarchy->Draw();

        hierarchy->Draw();


        EndDrawing();
    }
}

void Engine::Shutdown()
{
    if(inspector != nullptr){
        delete inspector;
        hierarchy = nullptr;
    }

    if (hierarchy != nullptr){
        delete hierarchy;
        hierarchy = nullptr;
    }

    CloseWindow();
}