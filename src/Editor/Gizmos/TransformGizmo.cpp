#include "TransformGizmo.h"
#include <algorithm>
#include <cmath>

TransformGizmo::TransformGizmo()
    : activeAxis(Axis::None),
      dragging(false),
      dragStartMouse{0.0f, 0.0f},
      dragStartPivot{0.0f, 0.0f},
      currentPivot{0.0f, 0.0f}
{
}

Vector2 TransformGizmo::CalculateGroupPivot(
    const std::vector<GameObject*>& objects
) const
{
    Vector2 pivot{0.0f, 0.0f};
    int count = 0;

    for (GameObject* object : objects)
    {
        if (object == nullptr)
        {
            continue;
        }

        Vector2 position = object->GetWorldPosition();

        pivot.x += position.x;
        pivot.y += position.y;
        ++count;
    }

    if (count == 0)
    {
        return pivot;
    }

    pivot.x /= static_cast<float>(count);
    pivot.y /= static_cast<float>(count);

    return pivot;
}

bool TransformGizmo::Update(
    Scene& scene,
    Vector2 mouseWorld,
    float cameraZoom,
    bool allowInteraction
)
{
    if (cameraZoom < 0.001f)
    {
        cameraZoom = 1.0f;
    }

    std::vector<GameObject*> selectedObjects =
        scene.GetSelectedObjects();

    if (selectedObjects.empty())
    {
        GameObject* selected = scene.GetSelectedObject();

        if (selected != nullptr)
        {
            selectedObjects.push_back(selected);
        }
    }

    selectedObjects.erase(
        std::remove(
            selectedObjects.begin(),
            selectedObjects.end(),
            nullptr
        ),
        selectedObjects.end()
    );

    if (selectedObjects.empty())
    {
        dragging = false;
        activeAxis = Axis::None;
        dragStartPositions.clear();

        return false;
    }

    currentPivot = CalculateGroupPivot(selectedObjects);

    if (!dragging &&
        allowInteraction &&
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        activeAxis = PickAxis(
            currentPivot,
            mouseWorld,
            cameraZoom
        );

        if (activeAxis != Axis::None)
        {
            dragging = true;
            dragStartMouse = mouseWorld;
            dragStartPivot = currentPivot;

            dragStartPositions.clear();
            dragStartPositions.reserve(selectedObjects.size());

            for (GameObject* object : selectedObjects)
            {
                dragStartPositions.push_back({
                    object,
                    object->GetWorldPosition()
                });
            }

            return true;
        }
    }

    if (dragging)
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 delta{
                mouseWorld.x - dragStartMouse.x,
                mouseWorld.y - dragStartMouse.y
            };

            if (activeAxis == Axis::X)
            {
                delta.y = 0.0f;
            }
            else if (activeAxis == Axis::Y)
            {
                delta.x = 0.0f;
            }

            for (const ObjectStartPosition& start :
                 dragStartPositions)
            {
                if (start.object == nullptr)
                {
                    continue;
                }

                Vector2 desiredPosition{
                    start.worldPosition.x + delta.x,
                    start.worldPosition.y + delta.y
                };

                SetWorldPosition(
                    start.object,
                    desiredPosition
                );
            }

            currentPivot = {
                dragStartPivot.x + delta.x,
                dragStartPivot.y + delta.y
            };

            return true;
        }

        dragging = false;
        activeAxis = Axis::None;
        dragStartPositions.clear();

        return true;
    }

    return false;
}

void TransformGizmo::Draw(
    Scene& scene,
    float cameraZoom
)
{
    if (cameraZoom < 0.001f)
    {
        cameraZoom = 1.0f;
    }

    std::vector<GameObject*> selectedObjects =
        scene.GetSelectedObjects();

    if (selectedObjects.empty())
    {
        GameObject* selected = scene.GetSelectedObject();

        if (selected != nullptr)
        {
            selectedObjects.push_back(selected);
        }
    }

    selectedObjects.erase(
        std::remove(
            selectedObjects.begin(),
            selectedObjects.end(),
            nullptr
        ),
        selectedObjects.end()
    );

    if (selectedObjects.empty())
    {
        return;
    }

    Vector2 pivot = dragging
        ? currentPivot
        : CalculateGroupPivot(selectedObjects);

    float length = 80.0f / cameraZoom;
    float thickness = 3.0f / cameraZoom;
    float arrowSize = 12.0f / cameraZoom;

    Vector2 xEnd{
        pivot.x + length,
        pivot.y
    };

    Vector2 yEnd{
        pivot.x,
        pivot.y + length
    };

    Color xColor =
        activeAxis == Axis::X ? YELLOW : RED;

    Color yColor =
        activeAxis == Axis::Y ? YELLOW : GREEN;

    DrawLineEx(
        pivot,
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
        pivot,
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
        pivot,
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
    Vector2 segment{
        end.x - start.x,
        end.y - start.y
    };

    Vector2 relative{
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

    Vector2 closestPoint{
        start.x + projection * segment.x,
        start.y + projection * segment.y
    };

    float dx = point.x - closestPoint.x;
    float dy = point.y - closestPoint.y;

    return sqrtf(dx * dx + dy * dy);
}

TransformGizmo::Axis TransformGizmo::PickAxis(
    Vector2 pivot,
    Vector2 mouseWorld,
    float cameraZoom
) const
{
    if (cameraZoom < 0.001f)
    {
        cameraZoom = 1.0f;
    }

    float length = 80.0f / cameraZoom;
    float threshold = 10.0f / cameraZoom;

    Vector2 xEnd{
        pivot.x + length,
        pivot.y
    };

    Vector2 yEnd{
        pivot.x,
        pivot.y + length
    };

    float xDistance = DistanceToSegment(
        mouseWorld,
        pivot,
        xEnd
    );

    float yDistance = DistanceToSegment(
        mouseWorld,
        pivot,
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
        object->GetTransform().SetPosition(worldPosition);
        return;
    }

    Vector2 localPosition =
        parent->InverseTransformPoint(worldPosition);

    object->GetTransform().SetPosition(localPosition);
}