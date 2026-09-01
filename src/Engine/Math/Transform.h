#pragma once

#include "raylib.h"

class TransformComponent{
    public:
        TransformComponent();

        //position
        void SetPosition(Vector2 position);
        Vector2 GetPosition() const;

        //rotation
        void SetRotation(float rotation);
        float GetRotation() const;

        //scale
        void SetScale(Vector2 scale);
        Vector2 GetScale() const;

        //movement
        void Translate(Vector2 amount);

        //rotation
        void Rotate(float amount);

    private:
        Vector2 position;
        float rotation;
        Vector2 scale;
};