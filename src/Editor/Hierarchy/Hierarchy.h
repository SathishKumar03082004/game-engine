#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"
#include "../History/EditorHistory.h"
#include <unordered_set>
#include <string>

class Hierarchy
{
public:
    explicit Hierarchy(Scene& sceneReference, EditorHistory& historyReference);

    void Initialize();
    void Update();
    void Draw();

    bool IsMouseOver() const;

private:
    Scene& scene;
    EditorHistory& history;

    Rectangle panel;
    Rectangle createButton;
    Rectangle deleteButton;

    float width;
    float height;

    std::unordered_set<GameObject*> collapsedObjects;

    bool isRenaming;
    GameObject* renamingObject;
    std::string renameBuffer;

    void CreateObject();
    void DeleteSelectedObject();
    void DuplicateSelectedObject();

    void StartRenaming();
    void CommitRenaming();
    void CancelRenaming();
    void HandleRenameInput();

    void DrawObject(GameObject* object, int depth, float& y);

    bool HandleObjectClick(
        GameObject* object,
        int depth,
        float& y,
        Vector2 mouse
    );

    bool IsExpanded(GameObject* object) const;
    void ToggleExpanded(GameObject* object);
};