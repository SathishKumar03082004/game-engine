#include "Scene.h"

Scene::Scene()
{
    selectedObject = nullptr;

    gridSize = 64;

    gridExtent = 15000;
}

void Scene::Initialize()
{
    GameObject player;

    player.SetName("Player");

    player.GetTransform().SetPosition({200.0f, 200.0f});

    player.GetTransform().SetRotation(45.0f);

    player.GetTransform().SetScale({1.5f, 1.5f});

    gameObjects.push_back(player);

    GameObject enemy;

    enemy.SetName("Enemy");

    enemy.GetTransform().SetPosition({500.0f, 300.0f});

    gameObjects.push_back(enemy);

    GameObject cameraObject;

    cameraObject.SetName("Camera");

    cameraObject.GetTransform().SetPosition({700.0f, 500.0f});

    gameObjects.push_back(cameraObject);
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
    for (int x = -gridExtent;x <= gridExtent;x += gridSize){
        DrawLine(x,-gridExtent,x,gridExtent,LIGHTGRAY);
    }

    for (int y = -gridExtent;y <= gridExtent;y += gridSize){
        DrawLine(-gridExtent,y,gridExtent,y,LIGHTGRAY);
    }

    DrawLine(-gridExtent,0,gridExtent,0,RED);

    DrawLine(0,-gridExtent,0,gridExtent,BLUE);
}

void Scene::SelectObject(Vector2 worldPosition)
{
    GameObject* objectToSelect = nullptr;

    for (auto it = gameObjects.rbegin();it != gameObjects.rend();++it){
        if (it->ContainsPoint(worldPosition))
        {
            objectToSelect = &(*it);
            break;
        }
    }

    SelectObject(objectToSelect);
}

void Scene::SelectObject(GameObject* object)
{
    if (selectedObject != nullptr)
    {
        selectedObject->SetSelected(false);
    }

    selectedObject = object;

    if (selectedObject != nullptr)
    {
        selectedObject->SetSelected(true);
    }
}

GameObject* Scene::GetSelectedObject()
{
    return selectedObject;
}

std::vector<GameObject>& Scene::GetGameObjects()
{
    return gameObjects;
}

GameObject* Scene::CreateGameObject(const std::string& requestedName){
    std::string finalName = requestedName;
    int counter = 1;
    bool nameExists = true;

    while (nameExists)
    {
        nameExists = false;
        for (GameObject& object : gameObjects)
        {
            if (object.GetName() == finalName)
            {
                nameExists = true;
                break;
            }
        }

        if (nameExists)
        {
            finalName = requestedName +" " +std::to_string(counter);
            counter++;
        }
    }

    GameObject newObject;

    newObject.SetName(finalName);

    float positionX = 400.0f + static_cast<float>(gameObjects.size() * 50);

    float positionY = 400.0f + static_cast<float>(gameObjects.size() * 30);

    newObject.GetTransform().SetPosition({positionX,positionY});

    gameObjects.push_back(newObject);

    GameObject* createdObject = &gameObjects.back();

    SelectObject(createdObject);

    return createdObject;
}

void Scene::DestroyGameObject(GameObject* object){
    if (object == nullptr)
    {
        return;
    }

    for (auto it = gameObjects.begin();it != gameObjects.end();++it){
        if (&(*it) == object)
        {
            if (selectedObject == object)
            {
                selectedObject = nullptr;
            }

            gameObjects.erase(it);
            break;
        }
    }

    if (selectedObject == nullptr)
    {
        for (GameObject& gameObject : gameObjects)
        {
            gameObject.SetSelected(false);
        }
    }
}

void Scene::DragSelectedObject(Vector2 worldPosition){
    if (selectedObject == nullptr)
    {
        return;
    }

    selectedObject->GetTransform().SetPosition(worldPosition);
}