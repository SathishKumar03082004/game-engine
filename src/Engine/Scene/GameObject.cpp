#include "GameObject.h"

#include <algorithm>
#include <cmath>

namespace
{
    Vector2 RotateVector(Vector2 vector, float degrees)
    {
        const float radians = degrees * DEG2RAD;
        const float cosine = cosf(radians);
        const float sine = sinf(radians);

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
    const Vector2 position = GetWorldPosition();
    const Vector2 worldScale = GetWorldScale();
    const float rotation = GetWorldRotation();

    float width = 100.0f * fabsf(worldScale.x);
    float height = 100.0f * fabsf(worldScale.y);

    // If a sprite exists, draw it using the GameObject transform.
    if (spriteRenderer.IsLoaded())
    {
        spriteRenderer.Draw(
            position,
            worldScale,
            rotation
        );

        width =
            static_cast<float>(spriteRenderer.GetWidth()) *
            fabsf(worldScale.x);

        height =
            static_cast<float>(spriteRenderer.GetHeight()) *
            fabsf(worldScale.y);
    }
    else
    {
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

        const Color objectColor = selected ? SKYBLUE : GRAY;

        DrawRectanglePro(
            rectangle,
            origin,
            rotation,
            objectColor
        );
    }

    // Draw the selection outline for both sprites and rectangles.
    if (selected)
    {
        Vector2 corners[4] = {
            {-width * 0.5f, -height * 0.5f},
            { width * 0.5f, -height * 0.5f},
            { width * 0.5f,  height * 0.5f},
            {-width * 0.5f,  height * 0.5f}
        };

        Vector2 worldCorners[4];

        for (int i = 0; i < 4; ++i)
        {
            const Vector2 rotated = RotateVector(
                corners[i],
                rotation
            );

            worldCorners[i] = {
                position.x + rotated.x,
                position.y + rotated.y
            };
        }

        for (int i = 0; i < 4; ++i)
        {
            const int next = (i + 1) % 4;

            DrawLineEx(
                worldCorners[i],
                worldCorners[next],
                3.0f,
                BLUE
            );
        }
    }

    // Draw children after the parent.
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

SpriteRenderer& GameObject::GetSpriteRenderer()
{
    return spriteRenderer;
}

const SpriteRenderer& GameObject::GetSpriteRenderer() const
{
    return spriteRenderer;
}

bool GameObject::SetSprite(const std::string& filePath)
{
    return spriteRenderer.LoadSprite(filePath);
}

void GameObject::ClearSprite()
{
    spriteRenderer.UnloadSprite();
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

    return parent->GetWorldRotation() +
           transform.GetRotation();
}

Vector2 GameObject::GetWorldScale() const
{
    const Vector2 localScale = transform.GetScale();

    if (parent == nullptr)
    {
        return localScale;
    }

    const Vector2 parentScale = parent->GetWorldScale();

    return {
        parentScale.x * localScale.x,
        parentScale.y * localScale.y
    };
}

Vector2 GameObject::TransformPoint(Vector2 localPoint) const
{
    const Vector2 worldScale = GetWorldScale();

    const Vector2 scaledPoint = {
        localPoint.x * worldScale.x,
        localPoint.y * worldScale.y
    };

    const Vector2 rotatedPoint = RotateVector(
        scaledPoint,
        GetWorldRotation()
    );

    const Vector2 worldPosition = GetWorldPosition();

    return {
        worldPosition.x + rotatedPoint.x,
        worldPosition.y + rotatedPoint.y
    };
}

Vector2 GameObject::InverseTransformPoint(Vector2 worldPoint) const
{
    const Vector2 worldPosition = GetWorldPosition();

    const Vector2 relativePoint = {
        worldPoint.x - worldPosition.x,
        worldPoint.y - worldPosition.y
    };

    const Vector2 unrotatedPoint = RotateVector(
        relativePoint,
        -GetWorldRotation()
    );

    const Vector2 worldScale = GetWorldScale();

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
    const Vector2 localPoint = InverseTransformPoint(worldPoint);

    const float halfWidth =
        spriteRenderer.IsLoaded()
            ? static_cast<float>(spriteRenderer.GetWidth()) * 0.5f
            : size.x * 0.5f;

    const float halfHeight =
        spriteRenderer.IsLoaded()
            ? static_cast<float>(spriteRenderer.GetHeight()) * 0.5f
            : size.y * 0.5f;

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

    // Prevent hierarchy cycles.
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

    const auto iterator = std::find(
        children.begin(),
        children.end(),
        child
    );

    if (iterator != children.end())
    {
        return;
    }

    children.push_back(child);
}

void GameObject::RemoveChild(GameObject* child)
{
    const auto iterator = std::find(
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