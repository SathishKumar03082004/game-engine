#pragma once

#include "raylib.h"
#include "GameObject.h"

#include <vector>
#include <string>

class Scene
{
public:

    Scene();

    void Initialize();
    void Update();
    void Draw();

    void SelectObject(Vector2 worldPosition);

    void SelectObject(GameObject* object);

    GameObject* GetSelectedObject();

    std::vector<GameObject>& GetGameObjects();

    GameObject* CreateGameObject(const std::string& name = "GameObject");

    void DestroyGameObject(GameObject* object);

    void DragSelectedObject(Vector2 worldPosition);


private:

    void DrawGrid();

    std::vector<GameObject> gameObjects;

    GameObject* selectedObject;

    int gridSize;

    int gridExtent;
};