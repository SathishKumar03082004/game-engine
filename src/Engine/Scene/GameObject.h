#pragma once

#include "raylib.h"
#include "../Math/Transform.h"

#include <string>

class GameObject
{
public:

    GameObject();

    void Update();
    void Draw();

    // Transform
    TransformComponent& GetTransform();

    // Selection
    void SetSelected(bool selected);
    bool IsSelected() const;

    bool ContainsPoint(Vector2 worldPoint) const;

    // Name
    void SetName(const std::string& newName);
    const std::string& GetName() const;

private:

    TransformComponent transform;

    bool selected;

    Vector2 size;

    std::string name;
};