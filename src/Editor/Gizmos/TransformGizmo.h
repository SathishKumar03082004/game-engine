#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"

class TransformGizmo
{
public:
    TransformGizmo();

    bool Update(
        Scene& scene,
        Vector2 mouseWorld,
        float cameraZoom,
        bool allowInteraction
    );

    void Draw(Scene& scene, float cameraZoom);

    bool IsDragging() const;

private:
    enum class Axis
    {
        None,
        X,
        Y
    };

    Axis activeAxis;
    bool dragging;

    Vector2 dragStartMouse;
    Vector2 dragStartPosition;

    float DistanceToSegment(
        Vector2 point,
        Vector2 start,
        Vector2 end
    ) const;

    Axis PickAxis(
        GameObject* object,
        Vector2 mouseWorld,
        float cameraZoom
    ) const;

    void SetWorldPosition(
        GameObject* object,
        Vector2 worldPosition
    );
};