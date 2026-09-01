#pragma once

#include "raylib.h"
#include "../Math/Transform.h"

class GameObject{
    public:
        GameObject();

        void Update();
        void Draw();

        TransformComponent& GetTransform();

        // void SetPosition(Vector2 position);
        // void SetRotation(float rotation);
        // void SetScale(Vector2 scale);

        // Vector2 GetPosition() const;
        // float GetRotation() const;
        // Vector2 GetScale() const;

        void SetSelected(bool selected);
        bool IsSelected() const;


    private:
        TransformComponent transform;

        bool selected;

        Vector2 size;

        // Vector2 position;
        // float rotation;
        // Vector2 scale;

        // bool selected;

        // Vector2 size;
};