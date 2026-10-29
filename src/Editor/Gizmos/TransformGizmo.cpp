#include "TransformGizmo.h"

#include <algorithm>
#include <cmath>

TransformGizmo::TransformGizmo()
    : activeAxis(Axis::None),
      dragging(false),
      dragStartMouse{0.0f, 0.0f},
      dragStartPosition{0.0f, 0.0f}
{
}

bool TransformGizmo::Update(
    Scene& scene,
    Vector2 mouseWorld,
    float cameraZoom,
    bool allowInteraction
)
{
    GameObject* object = scene.GetSelectedObject();

    if (object == nullptr)
    {
        dragging = false;
        activeAxis = Axis::None;
        return false;
    }

    if (cameraZoom < 0.001f)
    {
        cameraZoom = 1.0f;
    }

    if (!dragging &&
        allowInteraction &&
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        activeAxis = PickAxis(
            object,
            mouseWorld,
            cameraZoom
        );

        if (activeAxis != Axis::None)
        {
            dragging = true;
            dragStartMouse = mouseWorld;
            dragStartPosition = object->GetWorldPosition();

            return true;
        }
    }

    if (dragging)
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 desiredPosition = dragStartPosition;

            if (activeAxis == Axis::X)
            {
                desiredPosition.x +=
                    mouseWorld.x - dragStartMouse.x;
            }
            else if (activeAxis == Axis::Y)
            {
                desiredPosition.y +=
                    mouseWorld.y - dragStartMouse.y;
            }

            SetWorldPosition(
                object,
                desiredPosition
            );

            return true;
        }

        dragging = false;
        activeAxis = Axis::None;

        return true;
    }

    return false;
}

void TransformGizmo::Draw(
    Scene& scene,
    float cameraZoom
)
{
    GameObject* object = scene.GetSelectedObject();

    if (object == nullptr)
    {
        return;
    }

    if (cameraZoom < 0.001f)
    {
        cameraZoom = 1.0f;
    }

    Vector2 position = object->GetWorldPosition();

    float length = 80.0f / cameraZoom;
    float thickness = 3.0f / cameraZoom;
    float arrowSize = 12.0f / cameraZoom;

    Vector2 xEnd = {
        position.x + length,
        position.y
    };

    Vector2 yEnd = {
        position.x,
        position.y + length
    };

    Color xColor =
        activeAxis == Axis::X ? YELLOW : RED;

    Color yColor =
        activeAxis == Axis::Y ? YELLOW : GREEN;

    DrawLineEx(
        position,
        xEnd,
        thickness,
        xColor
    );

    DrawTriangle(
        xEnd,
        {
            xEnd.x - arrowSize,
            xEnd.y - arrowSize * 0.6f
        },
        {
            xEnd.x - arrowSize,
            xEnd.y + arrowSize * 0.6f
        },
        xColor
    );

    DrawLineEx(
        position,
        yEnd,
        thickness,
        yColor
    );

    DrawTriangle(
        yEnd,
        {
            yEnd.x - arrowSize * 0.6f,
            yEnd.y - arrowSize
        },
        {
            yEnd.x + arrowSize * 0.6f,
            yEnd.y - arrowSize
        },
        yColor
    );

    DrawCircleV(
        position,
        5.0f / cameraZoom,
        WHITE
    );
}

bool TransformGizmo::IsDragging() const
{
    return dragging;
}

float TransformGizmo::DistanceToSegment(
    Vector2 point,
    Vector2 start,
    Vector2 end
) const
{
    Vector2 segment = {
        end.x - start.x,
        end.y - start.y
    };

    Vector2 relative = {
        point.x - start.x,
        point.y - start.y
    };

    float lengthSquared =
        segment.x * segment.x +
        segment.y * segment.y;

    if (lengthSquared <= 0.0001f)
    {
        float dx = point.x - start.x;
        float dy = point.y - start.y;

        return sqrtf(dx * dx + dy * dy);
    }

    float projection =
        (relative.x * segment.x +
         relative.y * segment.y) /
        lengthSquared;

    projection = std::clamp(
        projection,
        0.0f,
        1.0f
    );

    Vector2 closestPoint = {
        start.x + projection * segment.x,
        start.y + projection * segment.y
    };

    float dx = point.x - closestPoint.x;
    float dy = point.y - closestPoint.y;

    return sqrtf(dx * dx + dy * dy);
}

TransformGizmo::Axis TransformGizmo::PickAxis(
    GameObject* object,
    Vector2 mouseWorld,
    float cameraZoom
) const
{
    if (object == nullptr)
    {
        return Axis::None;
    }

    Vector2 position = object->GetWorldPosition();

    float length = 80.0f / cameraZoom;
    float threshold = 10.0f / cameraZoom;

    Vector2 xEnd = {
        position.x + length,
        position.y
    };

    Vector2 yEnd = {
        position.x,
        position.y + length
    };

    float xDistance = DistanceToSegment(
        mouseWorld,
        position,
        xEnd
    );

    float yDistance = DistanceToSegment(
        mouseWorld,
        position,
        yEnd
    );

    bool nearX = xDistance <= threshold;
    bool nearY = yDistance <= threshold;

    if (nearX && nearY)
    {
        return xDistance <= yDistance
            ? Axis::X
            : Axis::Y;
    }

    if (nearX)
    {
        return Axis::X;
    }

    if (nearY)
    {
        return Axis::Y;
    }

    return Axis::None;
}

void TransformGizmo::SetWorldPosition(
    GameObject* object,
    Vector2 worldPosition
)
{
    if (object == nullptr)
    {
        return;
    }

    GameObject* parent = object->GetParent();

    if (parent == nullptr)
    {
        object->GetTransform().SetPosition(
            worldPosition
        );

        return;
    }

    Vector2 localPosition =
        parent->InverseTransformPoint(worldPosition);

    object->GetTransform().SetPosition(
        localPosition
    );
}