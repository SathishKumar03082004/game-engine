#pragma once

#include "raylib.h"
#include "GameObject.h"

#include <vector>
#include <memory>
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

    std::vector<std::unique_ptr<GameObject>>& GetGameObjects();

    GameObject* CreateGameObject(
        const std::string& name = "GameObject"
    );

    GameObject* DuplicateGameObject(
        GameObject* object
    );

    void DestroyGameObject(
        GameObject* object
    );

    void SetParent(
        GameObject* child,
        GameObject* parent
    );

    void ClearParent(
        GameObject* child
    );

    void DragSelectedObject(
        Vector2 worldPosition
    );

private:
    void DrawGrid();

    bool WouldCreateCycle(
        GameObject* child,
        GameObject* potentialParent
    ) const;

    GameObject* DuplicateRecursive(
        GameObject* source,
        GameObject* parent
    );

    std::string GenerateDuplicateName(
        const std::string& originalName
    ) const;

    std::vector<std::unique_ptr<GameObject>> gameObjects;

    GameObject* selectedObject;

    int gridSize;
    int gridExtent;
};