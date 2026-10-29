#include "TransformGizmo.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr float PI_VALUE = 3.14159265358979323846f;

    float GetAngle(Vector2 center, Vector2 point)
    {
        return atan2f(
            point.y - center.y,
            point.x - center.x
        );
    }

    float GetDistance(Vector2 a, Vector2 b)
    {
        float dx = b.x - a.x;
        float dy = b.y - a.y;

        return sqrtf(dx * dx + dy * dy);
    }

    Vector2 RotatePoint(Vector2 point, float angle)
    {
        float cosine = cosf(angle);
        float sine = sinf(angle);

        return {
            point.x * cosine - point.y * sine,
            point.x * sine + point.y * cosine
        };
    }
}

TransformGizmo::TransformGizmo()
    : mode(Mode::Move),
      activeAxis(Axis::None),
      dragging(false),
      dragStartMouse{0.0f, 0.0f},
      dragStartPivot{0.0f, 0.0f},
      currentPivot{0.0f, 0.0f},
      dragStartAngle(0.0f),
      dragStartDistance(1.0f)
{
}

std::vector<GameObject*> TransformGizmo::GetSelection(
    Scene& scene
) const
{
    std::vector<GameObject*> objects =
        scene.GetSelectedObjects();

    objects.erase(
        std::remove(
            objects.begin(),
            objects.end(),
            nullptr
        ),
        objects.end()
    );

    if (objects.empty())
    {
        GameObject* selected = scene.GetSelectedObject();

        if (selected != nullptr)
        {
            objects.push_back(selected);
        }
    }

    return objects;
}

Vector2 TransformGizmo::CalculatePivot(
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

    if (IsKeyPressed(KEY_W) && !dragging)
    {
        mode = Mode::Move;
    }

    if (IsKeyPressed(KEY_E) && !dragging)
    {
        mode = Mode::Rotate;
    }

    if (IsKeyPressed(KEY_R) && !dragging)
    {
        mode = Mode::Scale;
    }

    std::vector<GameObject*> objects = GetSelection(scene);

    if (objects.empty())
    {
        dragging = false;
        activeAxis = Axis::None;
        initialTransforms.clear();

        return false;
    }

    currentPivot = CalculatePivot(objects);

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

            dragStartAngle = GetAngle(
                currentPivot,
                mouseWorld
            );

            dragStartDistance = GetDistance(
                currentPivot,
                mouseWorld
            );

            if (dragStartDistance < 0.001f)
            {
                dragStartDistance = 0.001f;
            }

            initialTransforms.clear();
            initialTransforms.reserve(objects.size());

            for (GameObject* object : objects)
            {
                TransformComponent& transform =
                    object->GetTransform();

                initialTransforms.push_back({
                    object,
                    object->GetWorldPosition(),
                    transform.GetRotation(),
                    transform.GetScale()
                });
            }

            return true;
        }
    }

    if (!dragging)
    {
        return false;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        if (mode == Mode::Move)
        {
            ApplyMove(mouseWorld);
        }
        else if (mode == Mode::Rotate)
        {
            ApplyRotate(mouseWorld);
        }
        else if (mode == Mode::Scale)
        {
            ApplyScale(mouseWorld);
        }

        return true;
    }

    dragging = false;
    activeAxis = Axis::None;
    initialTransforms.clear();

    return true;
}

void TransformGizmo::ApplyMove(Vector2 mouseWorld)
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

    for (const ObjectTransform& initial : initialTransforms)
    {
        if (initial.object == nullptr)
        {
            continue;
        }

        Vector2 position{
            initial.position.x + delta.x,
            initial.position.y + delta.y
        };

        SetWorldPosition(initial.object, position);
    }

    currentPivot = {
        dragStartPivot.x + delta.x,
        dragStartPivot.y + delta.y
    };
}

void TransformGizmo::ApplyRotate(Vector2 mouseWorld)
{
    float currentAngle = GetAngle(
        dragStartPivot,
        mouseWorld
    );

    float deltaAngle = currentAngle - dragStartAngle;

    float deltaDegrees = deltaAngle * 180.0f / PI_VALUE;

    for (const ObjectTransform& initial : initialTransforms)
    {
        if (initial.object == nullptr)
        {
            continue;
        }

        Vector2 offset{
            initial.position.x - dragStartPivot.x,
            initial.position.y - dragStartPivot.y
        };

        Vector2 rotatedOffset = RotatePoint(
            offset,
            deltaAngle
        );

        Vector2 newPosition{
            dragStartPivot.x + rotatedOffset.x,
            dragStartPivot.y + rotatedOffset.y
        };

        SetWorldPosition(initial.object, newPosition);

        initial.object->GetTransform().SetRotation(
            initial.rotation + deltaDegrees
        );
    }
}

void TransformGizmo::ApplyScale(Vector2 mouseWorld)
{
    float currentDistance = GetDistance(
        dragStartPivot,
        mouseWorld
    );

    float factor = currentDistance / dragStartDistance;

    if (factor < 0.01f)
    {
        factor = 0.01f;
    }

    for (const ObjectTransform& initial : initialTransforms)
    {
        if (initial.object == nullptr)
        {
            continue;
        }

        Vector2 offset{
            initial.position.x - dragStartPivot.x,
            initial.position.y - dragStartPivot.y
        };

        Vector2 newPosition{
            dragStartPivot.x + offset.x * factor,
            dragStartPivot.y + offset.y * factor
        };

        SetWorldPosition(initial.object, newPosition);

        Vector2 newScale{
            initial.scale.x * factor,
            initial.scale.y * factor
        };

        initial.object->GetTransform().SetScale(newScale);
    }
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

    std::vector<GameObject*> objects = GetSelection(scene);

    if (objects.empty())
    {
        return;
    }

    Vector2 pivot = dragging
        ? currentPivot
        : CalculatePivot(objects);

    float size = 80.0f / cameraZoom;
    float thickness = 3.0f / cameraZoom;
    float radius = 55.0f / cameraZoom;

    if (mode == Mode::Move)
    {
        Vector2 xEnd{
            pivot.x + size,
            pivot.y
        };

        Vector2 yEnd{
            pivot.x,
            pivot.y + size
        };

        Color xColor =
            activeAxis == Axis::X ? YELLOW : RED;

        Color yColor =
            activeAxis == Axis::Y ? YELLOW : GREEN;

        DrawLineEx(pivot, xEnd, thickness, xColor);
        DrawLineEx(pivot, yEnd, thickness, yColor);

        DrawTriangle(
            xEnd,
            {
                xEnd.x - size * 0.15f,
                xEnd.y - size * 0.08f
            },
            {
                xEnd.x - size * 0.15f,
                xEnd.y + size * 0.08f
            },
            xColor
        );

        DrawTriangle(
            yEnd,
            {
                yEnd.x - size * 0.08f,
                yEnd.y - size * 0.15f
            },
            {
                yEnd.x + size * 0.08f,
                yEnd.y - size * 0.15f
            },
            yColor
        );
    }
    else if (mode == Mode::Rotate)
    {
        DrawCircleLinesV(
            pivot,
            radius,
            activeAxis == Axis::Center ? YELLOW : BLUE
        );
    }
    else if (mode == Mode::Scale)
    {
        Vector2 xEnd{
            pivot.x + size,
            pivot.y
        };

        Vector2 yEnd{
            pivot.x,
            pivot.y + size
        };

        DrawLineEx(pivot, xEnd, thickness, RED);
        DrawLineEx(pivot, yEnd, thickness, GREEN);

        DrawRectangle(
            static_cast<int>(xEnd.x - 5.0f / cameraZoom),
            static_cast<int>(xEnd.y - 5.0f / cameraZoom),
            static_cast<int>(10.0f / cameraZoom),
            static_cast<int>(10.0f / cameraZoom),
            RED
        );

        DrawRectangle(
            static_cast<int>(yEnd.x - 5.0f / cameraZoom),
            static_cast<int>(yEnd.y - 5.0f / cameraZoom),
            static_cast<int>(10.0f / cameraZoom),
            static_cast<int>(10.0f / cameraZoom),
            GREEN
        );
    }

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
        return GetDistance(point, start);
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

    Vector2 closest{
        start.x + projection * segment.x,
        start.y + projection * segment.y
    };

    return GetDistance(point, closest);
}

TransformGizmo::Axis TransformGizmo::PickAxis(
    Vector2 pivot,
    Vector2 mouse,
    float zoom
) const
{
    float threshold = 10.0f / zoom;
    float size = 80.0f / zoom;

    if (mode == Mode::Rotate)
    {
        float distance = GetDistance(pivot, mouse);
        float radius = 55.0f / zoom;

        if (fabsf(distance - radius) <= threshold)
        {
            return Axis::Center;
        }

        return Axis::None;
    }

    Vector2 xEnd{
        pivot.x + size,
        pivot.y
    };

    Vector2 yEnd{
        pivot.x,
        pivot.y + size
    };

    float xDistance = DistanceToSegment(
        mouse,
        pivot,
        xEnd
    );

    float yDistance = DistanceToSegment(
        mouse,
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