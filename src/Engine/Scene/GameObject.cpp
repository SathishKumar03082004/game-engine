#include "GameObject.h"

#include <algorithm>
#include <cmath>

namespace
{
    Vector2 RotateVector(Vector2 vector, float degrees)
    {
        float radians = degrees * DEG2RAD;

        float cosine = cosf(radians);
        float sine = sinf(radians);

        return {
            vector.x * cosine - vector.y * sine,
            vector.x * sine + vector.y * cosine
        };
    }

    float DivideSafely(float value, float divisor)
    {
        if (fabsf(divisor) < 0.0001f)
        {
            return 0.0f;
        }

        return value / divisor;
    }
}

GameObject::GameObject()
    : selected(false),
      size{100.0f, 100.0f},
      name("GameObject"),
      parent(nullptr)
{
}

void GameObject::Update()
{
    for (GameObject* child : children)
    {
        if (child != nullptr)
        {
            child->Update();
        }
    }
}

void GameObject::Draw()
{
    Vector2 position = GetWorldPosition();
    Vector2 worldScale = GetWorldScale();
    float rotation = GetWorldRotation();

    float width = fabsf(size.x * worldScale.x);
    float height = fabsf(size.y * worldScale.y);

    Rectangle rectangle = {
        position.x - width * 0.5f,
        position.y - height * 0.5f,
        width,
        height
    };

    Vector2 origin = {
        width * 0.5f,
        height * 0.5f
    };

    Color objectColor = selected ? SKYBLUE : GRAY;

    DrawRectanglePro(
        rectangle,
        origin,
        rotation,
        objectColor
    );

    if (selected)
    {
        Vector2 corners[4] = {
            {-width * 0.5f, -height * 0.5f},
            { width * 0.5f, -height * 0.5f},
            { width * 0.5f,  height * 0.5f},
            {-width * 0.5f,  height * 0.5f}
        };

        Vector2 worldCorners[4];

        for (int i = 0; i < 4; i++)
        {
            Vector2 rotated = RotateVector(corners[i], rotation);

            worldCorners[i] = {
                position.x + rotated.x,
                position.y + rotated.y
            };
        }

        for (int i = 0; i < 4; i++)
        {
            int next = (i + 1) % 4;

            DrawLineEx(
                worldCorners[i],
                worldCorners[next],
                3.0f,
                BLUE
            );
        }
    }

    for (GameObject* child : children)
    {
        if (child != nullptr)
        {
            child->Draw();
        }
    }
}

TransformComponent& GameObject::GetTransform()
{
    return transform;
}

Vector2 GameObject::GetWorldPosition() const
{
    if (parent == nullptr)
    {
        return transform.GetPosition();
    }

    return parent->TransformPoint(transform.GetPosition());
}

float GameObject::GetWorldRotation() const
{
    if (parent == nullptr)
    {
        return transform.GetRotation();
    }

    return parent->GetWorldRotation() + transform.GetRotation();
}

Vector2 GameObject::GetWorldScale() const
{
    Vector2 localScale = transform.GetScale();

    if (parent == nullptr)
    {
        return localScale;
    }

    Vector2 parentScale = parent->GetWorldScale();

    return {
        parentScale.x * localScale.x,
        parentScale.y * localScale.y
    };
}

Vector2 GameObject::TransformPoint(Vector2 localPoint) const
{
    Vector2 worldScale = GetWorldScale();

    Vector2 scaledPoint = {
        localPoint.x * worldScale.x,
        localPoint.y * worldScale.y
    };

    Vector2 rotatedPoint = RotateVector(
        scaledPoint,
        GetWorldRotation()
    );

    Vector2 worldPosition = GetWorldPosition();

    return {
        worldPosition.x + rotatedPoint.x,
        worldPosition.y + rotatedPoint.y
    };
}

Vector2 GameObject::InverseTransformPoint(Vector2 worldPoint) const
{
    Vector2 worldPosition = GetWorldPosition();

    Vector2 relativePoint = {
        worldPoint.x - worldPosition.x,
        worldPoint.y - worldPosition.y
    };

    Vector2 unrotatedPoint = RotateVector(
        relativePoint,
        -GetWorldRotation()
    );

    Vector2 worldScale = GetWorldScale();

    return {
        DivideSafely(unrotatedPoint.x, worldScale.x),
        DivideSafely(unrotatedPoint.y, worldScale.y)
    };
}

void GameObject::SetSelected(bool value)
{
    selected = value;
}

bool GameObject::IsSelected() const
{
    return selected;
}

bool GameObject::ContainsPoint(Vector2 worldPoint) const
{
    Vector2 localPoint = InverseTransformPoint(worldPoint);

    float halfWidth = fabsf(size.x) * 0.5f;
    float halfHeight = fabsf(size.y) * 0.5f;

    return localPoint.x >= -halfWidth &&
           localPoint.x <= halfWidth &&
           localPoint.y >= -halfHeight &&
           localPoint.y <= halfHeight;
}

void GameObject::SetName(const std::string& newName)
{
    name = newName;
}

const std::string& GameObject::GetName() const
{
    return name;
}

void GameObject::SetParent(GameObject* newParent)
{
    if (newParent == this || newParent == parent)
    {
        return;
    }

    for (GameObject* ancestor = newParent;
         ancestor != nullptr;
         ancestor = ancestor->GetParent())
    {
        if (ancestor == this)
        {
            return;
        }
    }

    if (parent != nullptr)
    {
        parent->RemoveChild(this);
    }

    parent = newParent;

    if (parent != nullptr)
    {
        parent->AddChild(this);
    }
}

GameObject* GameObject::GetParent() const
{
    return parent;
}

void GameObject::AddChild(GameObject* child)
{
    if (child == nullptr || child == this)
    {
        return;
    }

    if (std::find(children.begin(), children.end(), child) != children.end())
    {
        return;
    }

    children.push_back(child);
}

void GameObject::RemoveChild(GameObject* child)
{
    auto iterator = std::find(
        children.begin(),
        children.end(),
        child
    );

    if (iterator != children.end())
    {
        children.erase(iterator);
    }
}

const std::vector<GameObject*>& GameObject::GetChildren() const
{
    return children;
}

bool GameObject::HasChildren() const
{
    return !children.empty();
}