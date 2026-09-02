#pragma once

#include "raylib.h"
#include "../Math/Transform.h"

class GameObject{
    public:
        GameObject();

        void Update();
        void Draw();

        TransformComponent& GetTransform();

        void SetSelected(bool selected);
        bool IsSelected() const;

        bool ContainsPoint(Vector2 worldPoint) const;


    private:
        TransformComponent transform;

        bool selected;

        Vector2 size;
};