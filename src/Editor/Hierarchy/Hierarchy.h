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
    Rectangle saveButton;

    float width;
    float height;

    std::unordered_set<GameObject*> collapsedObjects;

    bool isRenaming;
    GameObject* renamingObject;
    std::string renameBuffer;

    GameObject* dragCandidate;
    Vector2 dragStartPosition;
    bool isDraggingObject;
    std::string saveStatus;

    void CreateObject();
    void DeleteSelectedObject();
    void DuplicateSelectedObject();
    void SaveScene();

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

    GameObject* FindObjectAt(Vector2 mouse) const;
    GameObject* FindObjectAt(
        GameObject* object,
        int depth,
        float& y,
        Vector2 mouse
    ) const;
    void FinishObjectDrag(Vector2 mouse);

    bool IsExpanded(GameObject* object) const;
    void ToggleExpanded(GameObject* object);
};
