#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"
#include <vector>

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

    struct ObjectStartPosition
    {
        GameObject* object;
        Vector2 worldPosition;
    };

    Axis activeAxis;
    bool dragging;

    Vector2 dragStartMouse;
    Vector2 dragStartPivot;
    Vector2 currentPivot;

    std::vector<ObjectStartPosition> dragStartPositions;

    Vector2 CalculateGroupPivot(
        const std::vector<GameObject*>& objects
    ) const;

    float DistanceToSegment(
        Vector2 point,
        Vector2 start,
        Vector2 end
    ) const;

    Axis PickAxis(
        Vector2 pivot,
        Vector2 mouseWorld,
        float cameraZoom
    ) const;

    void SetWorldPosition(
        GameObject* object,
        Vector2 worldPosition
    );
};