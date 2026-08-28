#include "Scene.h"

void Scene::Initialize(){

}

void Scene::Update(){

}

void Scene::Draw(){
    DrawGrid();
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