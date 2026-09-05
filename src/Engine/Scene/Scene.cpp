#include "Scene.h"

Scene::Scene()
{
    selectedObject = nullptr;
}

void Scene::Initialize()
{
    GameObject object1;

    object1.GetTransform().SetPosition({200.0f,200.0f});

    object1.GetTransform().SetRotation(45.0f);

    object1.GetTransform().SetScale({1.5f,1.5f});

    gameObjects.push_back(object1);


    GameObject object2;

    object2.GetTransform().SetPosition({500.0f,300.0f});

    gameObjects.push_back(object2);


    GameObject object3;

    object3.GetTransform().SetPosition({700.0f,500.0f});

    gameObjects.push_back(object3);
}

void Scene::Update()
{
    for (GameObject& object : gameObjects)
    {
        object.Update();
    }
}

void Scene::Draw()
{
    DrawGrid();

    for (GameObject& object : gameObjects)
    {
        object.Draw();
    }
}

void Scene::DrawGrid()
{
    for (int x = -gridExtent;x <= gridExtent;x += gridSize)
    {
        DrawLine(x,-gridExtent,x,gridExtent,LIGHTGRAY);
    }

    for (int y = -gridExtent;y <= gridExtent;y += gridSize)
    {
        DrawLine(-gridExtent,y,gridExtent,y,LIGHTGRAY);
    }

    DrawLine(-gridExtent,0,gridExtent,0,RED);

    DrawLine(0,-gridExtent,0,gridExtent,BLUE);
}

void Scene::SelectObject(Vector2 worldPosition)
{
    if (selectedObject != nullptr)
    {
        selectedObject->SetSelected(false);
        selectedObject = nullptr;
    }


    for (auto it = gameObjects.rbegin();it != gameObjects.rend();++it)
    {
        if (it->ContainsPoint(worldPosition))
        {
            it->SetSelected(true);

            selectedObject = &(*it);

            break;
        }
    }
}

GameObject* Scene::GetSelectedObject()
{
    return selectedObject;
}

std::vector<GameObject>& Scene::GetGameObjects(){
    return gameObjects;
}

void Scene::DragSelectedObject(Vector2 worldPosition)
{
    if (selectedObject == nullptr)
        return;

    selectedObject->GetTransform().SetPosition(worldPosition);
}