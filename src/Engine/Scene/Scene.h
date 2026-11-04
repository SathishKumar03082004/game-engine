#pragma once

#include "raylib.h"
#include "GameObject.h"
#include "../Physics/PhysicsSystem.h"
#include <vector>
#include <memory>
#include <string>
#include <utility>

class Scene
{
public:
    Scene();

    void Initialize();
    void Update();
    void Draw();
    PhysicsSystem& GetPhysicsSystem();
    const PhysicsSystem& GetPhysicsSystem() const;

    GameObject* GetObjectAt(Vector2 worldPosition) const;
    void SelectObject(Vector2 worldPosition, bool ctrlPressed = false);
    void SelectObject(GameObject* object, bool ctrlPressed = false);
    void ClearSelection();
    void ToggleSelection(GameObject* object);

    GameObject* GetSelectedObject();
    const std::vector<GameObject*>& GetSelectedObjects() const;
    bool IsObjectSelected(GameObject* object) const;

    std::vector<std::unique_ptr<GameObject>>& GetGameObjects();

    GameObject* CreateGameObject(const std::string& name = "GameObject");
    GameObject* DuplicateGameObject(GameObject* object);
    void DestroyGameObject(GameObject* object);

    void SetParent(GameObject* child, GameObject* parent);
    void ClearParent(GameObject* child);

    void DragSelectedObject(Vector2 worldPosition);
    void BeginSelectionDrag(Vector2 worldPosition);
    void DragSelection(Vector2 worldPosition);
    void SetGridSize(int size);
    int GetGridSize() const;
    void SetGridSnappingEnabled(bool enabled);
    bool IsGridSnappingEnabled() const;
    Vector2 SnapToGrid(Vector2 worldPosition) const;
    bool SaveToFile(const std::string& path) const;
    bool LoadFromFile(const std::string& path);

private:
    void DrawGrid();

    bool WouldCreateCycle(
        GameObject* child,
        GameObject* potentialParent
    ) const;

    bool ContainsGameObject(GameObject* object) const;
    bool LoadJsonFromFile(const std::string& path);
    bool LoadLegacyFromFile(const std::string& path);

    GameObject* DuplicateRecursive(
        GameObject* source,
        GameObject* parent
    );

    std::string GenerateDuplicateName(
        const std::string& originalName
    ) const;

    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::vector<GameObject*> selectedObjects;
    GameObject* selectedObject;
    PhysicsSystem physicsSystem;

    int gridSize;
    int gridExtent;
    bool gridSnappingEnabled;
    Vector2 selectionDragMouseStart{};
    std::vector<std::pair<GameObject*, Vector2>> selectionDragStartPositions;
};
