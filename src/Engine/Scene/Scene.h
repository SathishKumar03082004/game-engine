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

    void SelectObject(Vector2 worldPosition);
    GameObject* GetSelectedObject();

private:
    void DrawGrid();

    std::vector<GameObject> gameObjects;

    GameObject* selectedObject;

    int gridSize = 64;
    int gridExtent = 15000;
};