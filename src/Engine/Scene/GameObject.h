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

    TransformComponent& GetTransform();

    Vector2 GetWorldPosition() const;
    float GetWorldRotation() const;
    Vector2 GetWorldScale() const;

    void SetSelected(bool selected);
    bool IsSelected() const;

    bool ContainsPoint(Vector2 worldPoint) const;

    void SetName(const std::string& newName);
    const std::string& GetName() const;

    void SetParent(GameObject* newParent);
    GameObject* GetParent() const;

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