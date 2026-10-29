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
    enum class Mode
    {
        Move,
        Rotate,
        Scale
    };

    enum class Axis
    {
        None,
        X,
        Y,
        Center
    };

    struct ObjectTransform
    {
        GameObject* object;
        Vector2 position;
        float rotation;
        Vector2 scale;
    };

    Mode mode;
    Axis activeAxis;
    bool dragging;

    Vector2 dragStartMouse;
    Vector2 dragStartPivot;
    Vector2 currentPivot;

    float dragStartAngle;
    float dragStartDistance;

    std::vector<ObjectTransform> initialTransforms;

    Vector2 CalculatePivot(
        const std::vector<GameObject*>& objects
    ) const;

    std::vector<GameObject*> GetSelection(Scene& scene) const;

    Axis PickAxis(
        Vector2 pivot,
        Vector2 mouse,
        float zoom
    ) const;

    float DistanceToSegment(
        Vector2 point,
        Vector2 start,
        Vector2 end
    ) const;

    void SetWorldPosition(
        GameObject* object,
        Vector2 position
    );

    void ApplyMove(Vector2 mouseWorld);
    void ApplyRotate(Vector2 mouseWorld);
    void ApplyScale(Vector2 mouseWorld);
};