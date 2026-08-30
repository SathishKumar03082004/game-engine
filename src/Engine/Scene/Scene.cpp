#include "Scene.h"

void Scene::Initialize(){
    gameObject.SetPosition({200.0f, 200.0f});
}

void Scene::Update(){
    gameObject.Update();
}

void Scene::Draw(){
    DrawGrid();

    gameObject.Draw();
}

void Scene::DrawGrid()
{
    for (int x = -gridExtent; x <= gridExtent; x += gridSize){
        DrawLine(x,-gridExtent,x,gridExtent,LIGHTGRAY);
    }

    for (int y = -gridExtent; y <= gridExtent; y += gridSize){
        DrawLine(-gridExtent,y,gridExtent,y,LIGHTGRAY);
    }

    DrawLine(-gridExtent,0,gridExtent,0,RED);

    DrawLine(0,-gridExtent,0,gridExtent,BLUE);
}