#pragma once

#include "raylib.h"
#include "GameObject.h"

#include <vector>

class Scene
{
public:
    Scene();

    void Initialize();
    void Update();
    void Draw();

    // Selection
    void SelectObject(Vector2 worldPosition);
    void SelectObject(GameObject* object);
    GameObject* GetSelectedObject();

    // Objects
    std::vector<GameObject>& GetGameObjects();

    // Object movement
    void DragSelectedObject(Vector2 worldPosition);

private:

    void DrawGrid();

    std::vector<GameObject> gameObjects;

    GameObject* selectedObject;

    int gridSize;
    int gridExtent;
};