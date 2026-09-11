#pragma once

#include "raylib.h"
#include "../Math/Transform.h"

#include <string>
#include <vector>

class GameObject
{
public:

    GameObject();

    void Update();
    void Draw();

    // =========================================================
    // TRANSFORM
    // =========================================================

    TransformComponent& GetTransform();

    Vector2 GetWorldPosition() const;
    float GetWorldRotation() const;
    Vector2 GetWorldScale() const;

    // =========================================================
    // SELECTION
    // =========================================================

    void SetSelected(bool selected);
    bool IsSelected() const;

    bool ContainsPoint(Vector2 worldPoint) const;

    // =========================================================
    // NAME
    // =========================================================

    void SetName(const std::string& newName);
    const std::string& GetName() const;

    // =========================================================
    // PARENT
    // =========================================================

    void SetParent(GameObject* newParent);

    GameObject* GetParent() const;

    // =========================================================
    // CHILDREN
    // =========================================================

    void AddChild(GameObject* child);

    void RemoveChild(GameObject* child);

    const std::vector<GameObject*>& GetChildren() const;

    bool HasChildren() const;

private:

    TransformComponent transform;

    bool selected;

    Vector2 size;

    std::string name;

    GameObject* parent;

    std::vector<GameObject*> children;
};