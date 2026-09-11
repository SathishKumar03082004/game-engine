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

    // =========================================================
    // SELECTION
    // =========================================================

    void SelectObject(Vector2 worldPosition);

    void SelectObject(GameObject* object);

    GameObject* GetSelectedObject();

    // =========================================================
    // GAME OBJECTS
    // =========================================================

    std::vector<std::unique_ptr<GameObject>>& GetGameObjects();

    GameObject* CreateGameObject(
        const std::string& name = "GameObject"
    );

    void DestroyGameObject(
        GameObject* object
    );

    // =========================================================
    // PARENTING
    // =========================================================

    void SetParent(
        GameObject* child,
        GameObject* parent
    );

    void ClearParent(
        GameObject* child
    );

    // =========================================================
    // DRAG
    // =========================================================

    void DragSelectedObject(
        Vector2 worldPosition
    );

private:

    void DrawGrid();

    bool WouldCreateCycle(
        GameObject* child,
        GameObject* potentialParent
    ) const;

private:

    std::vector<std::unique_ptr<GameObject>> gameObjects;

    GameObject* selectedObject;

    int gridSize;

    int gridExtent;
};