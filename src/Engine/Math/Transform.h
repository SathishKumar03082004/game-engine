#pragma once

#include "raylib.h"

class TransformComponent{
    public:
        TransformComponent();

        void SetPosition(Vector2 position);
        Vector2 GetPosition() const;

        void SetRotation(float rotation);
        float GetRotation() const;

        void SetScale(Vector2 scale);
        Vector2 GetScale() const;

        void Translate(Vector2 amount);

        void Rotate(float amount);

    private:
        Vector2 position;
        float rotation;
        Vector2 scale;
};