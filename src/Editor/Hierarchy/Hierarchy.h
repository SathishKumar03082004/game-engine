#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"

#include <unordered_set>
#include <string>

class Hierarchy
{
public:
    Hierarchy(Scene& sceneReference);

    void Initialize();
    void Update();
    void Draw();

    bool IsMouseOver() const;

private:
    Scene& scene;

    Rectangle panel;

    float width;
    float height;

    int selectedIndex;

    Rectangle createButton;
    Rectangle deleteButton;

    std::unordered_set<GameObject*> collapsedObjects;

    bool isRenaming;
    GameObject* renamingObject;
    std::string renameBuffer;

    void CreateObject();
    void DeleteSelectedObject();

    void StartRenaming();
    void CommitRenaming();
    void CancelRenaming();
    void HandleRenameInput();

    void DrawObject(
        GameObject* object,
        int depth,
        float& y
    );

    bool HandleObjectClick(
        GameObject* object,
        int depth,
        float& y,
        Vector2 mouse
    );

    void SelectObject(
        GameObject* object
    );

    bool IsExpanded(
        GameObject* object
    ) const;

    void ToggleExpanded(
        GameObject* object
    );
};