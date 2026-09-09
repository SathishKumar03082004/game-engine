#pragma once

#include "raylib.h"


struct TransformComponent
{
    TransformComponent();


    void SetPosition(Vector2 newPosition);
    Vector2 GetPosition() const;


    void SetRotation(float newRotation);
    float GetRotation() const;


    void SetScale(Vector2 newScale);
    Vector2 GetScale() const;


    void Translate(Vector2 amount);
    void Rotate(float amount);


    Vector2 position;

    float rotation;

    Vector2 scale;
};