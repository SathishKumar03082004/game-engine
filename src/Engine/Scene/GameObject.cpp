#include "GameObject.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float PI_VALUE = 3.14159265358979323846f;

    Vector2 RotateVector(Vector2 value, float degrees)
    {
        float radians = degrees * PI_VALUE / 180.0f;

        float cosine = std::cos(radians);
        float sine = std::sin(radians);

        return
        {
            value.x * cosine - value.y * sine,
            value.x * sine + value.y * cosine
        };
    }

    Vector2 DivideSafely(Vector2 value, Vector2 divisor)
    {
        constexpr float EPSILON = 0.000001f;

        return
        {
            std::fabs(divisor.x) > EPSILON
                ? value.x / divisor.x : 0.0f,

            std::fabs(divisor.y) > EPSILON
                ? value.y / divisor.y : 0.0f
        };
    }
}

GameObject::GameObject()
{
    selected = false;
    size = { 100.0f, 100.0f };
    name = "GameObject";
    parent = nullptr;
}

void GameObject::Update()
{
}

void GameObject::Draw()
{
    Vector2 position = GetWorldPosition();
    Vector2 scale = GetWorldScale();
    float rotation = GetWorldRotation();

    float width = size.x * scale.x;
    float height = size.y * scale.y;

    Rectangle rectangle =
    {
        position.x,
        position.y,
        width,
        height
    };

    Vector2 origin =
    {
        width / 2.0f,
        height / 2.0f
    };

    DrawRectanglePro(
        rectangle,
        origin,
        rotation,
        GRAY
    );

    if (selected)
    {
        DrawRectanglePro(
            rectangle,
            origin,
            rotation,
            BLANK
        );

        DrawRectangleLinesEx(
            {
                position.x - std::fabs(width) / 2.0f,
                position.y - std::fabs(height) / 2.0f,
                std::fabs(width),
                std::fabs(height)
            },
            3.0f,
            BLUE
        );
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
    Vector2 localPosition = transform.GetPosition();

    if (parent == nullptr)
    {
        return localPosition;
    }

    return parent->TransformPoint(localPosition);
}

float GameObject::GetWorldRotation() const
{
    float localRotation = transform.GetRotation();

    if (parent == nullptr)
    {
        return localRotation;
    }

    return parent->GetWorldRotation() + localRotation;
}

Vector2 GameObject::GetWorldScale() const
{
    Vector2 localScale = transform.GetScale();

    if (parent == nullptr)
    {
        return localScale;
    }

    Vector2 parentScale = parent->GetWorldScale();

    return
    {
        parentScale.x * localScale.x,
        parentScale.y * localScale.y
    };
}

Vector2 GameObject::TransformPoint(Vector2 localPoint) const
{
    Vector2 worldScale = GetWorldScale();

    Vector2 scaledPoint =
    {
        localPoint.x * worldScale.x,
        localPoint.y * worldScale.y
    };

    Vector2 rotatedPoint = RotateVector(
        scaledPoint,
        GetWorldRotation()
    );

    Vector2 worldPosition = GetWorldPosition();

    return
    {
        worldPosition.x + rotatedPoint.x,
        worldPosition.y + rotatedPoint.y
    };
}

Vector2 GameObject::InverseTransformPoint(Vector2 worldPoint) const
{
    Vector2 worldPosition = GetWorldPosition();

    Vector2 difference =
    {
        worldPoint.x - worldPosition.x,
        worldPoint.y - worldPosition.y
    };

    Vector2 unrotated = RotateVector(
        difference,
        -GetWorldRotation()
    );

    return DivideSafely(
        unrotated,
        GetWorldScale()
    );
}

void GameObject::SetSelected(bool newSelected)
{
    selected = newSelected;
}

bool GameObject::IsSelected() const
{
    return selected;
}

bool GameObject::ContainsPoint(Vector2 worldPoint) const
{
    Vector2 position = GetWorldPosition();
    Vector2 scale = GetWorldScale();

    float width = size.x * scale.x;
    float height = size.y * scale.y;

    Rectangle rectangle =
    {
        position.x - std::fabs(width) / 2.0f,
        position.y - std::fabs(height) / 2.0f,
        std::fabs(width),
        std::fabs(height)
    };

    return CheckCollisionPointRec(
        worldPoint,
        rectangle
    );
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
    if (parent == newParent)
    {
        return;
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

    auto it = std::find(
        children.begin(),
        children.end(),
        child
    );

    if (it != children.end())
    {
        return;
    }

    children.push_back(child);

    if (child->parent != this)
    {
        child->parent = this;
    }
}

void GameObject::RemoveChild(GameObject* child)
{
    if (child == nullptr)
    {
        return;
    }

    children.erase(
        std::remove(
            children.begin(),
            children.end(),
            child
        ),
        children.end()
    );

    if (child->parent == this)
    {
        child->parent = nullptr;
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