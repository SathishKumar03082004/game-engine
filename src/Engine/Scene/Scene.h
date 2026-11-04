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

    int gridSize;
    int gridExtent;
};
