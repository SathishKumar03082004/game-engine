#include "GameObject.h"

#include <algorithm>

// =========================================================
// CONSTRUCTOR
// =========================================================

GameObject::GameObject()
{
    selected = false;

    size = { 100.0f, 100.0f };

    name = "GameObject";

    parent = nullptr;
}

// =========================================================
// UPDATE
// =========================================================

void GameObject::Update()
{
}

// =========================================================
// DRAW
// =========================================================

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
        DrawRectangleLinesEx(
            rectangle,
            3.0f,
            BLUE
        );
    }
}

// =========================================================
// TRANSFORM
// =========================================================

TransformComponent& GameObject::GetTransform()
{
    return transform;
}

// =========================================================
// WORLD POSITION
// =========================================================

Vector2 GameObject::GetWorldPosition() const
{
    Vector2 localPosition = transform.GetPosition();

    if (parent == nullptr)
    {
        return localPosition;
    }

    Vector2 parentPosition = parent->GetWorldPosition();

    return
    {
        parentPosition.x + localPosition.x,
        parentPosition.y + localPosition.y
    };
}

// =========================================================
// WORLD ROTATION
// =========================================================

float GameObject::GetWorldRotation() const
{
    float localRotation = transform.GetRotation();

    if (parent == nullptr)
    {
        return localRotation;
    }

    return parent->GetWorldRotation() + localRotation;
}

// =========================================================
// WORLD SCALE
// =========================================================

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

// =========================================================
// SELECTION
// =========================================================

void GameObject::SetSelected(bool newSelected)
{
    selected = newSelected;
}

bool GameObject::IsSelected() const
{
    return selected;
}

// =========================================================
// COLLISION
// =========================================================

bool GameObject::ContainsPoint(Vector2 worldPoint) const
{
    Vector2 position = GetWorldPosition();

    Vector2 scale = GetWorldScale();

    float width = size.x * scale.x;
    float height = size.y * scale.y;

    Rectangle rectangle =
    {
        position.x - width / 2.0f,
        position.y - height / 2.0f,
        width,
        height
    };

    return CheckCollisionPointRec(
        worldPoint,
        rectangle
    );
}

// =========================================================
// NAME
// =========================================================

void GameObject::SetName(const std::string& newName)
{
    name = newName;
}

const std::string& GameObject::GetName() const
{
    return name;
}

// =========================================================
// SET PARENT
// =========================================================

void GameObject::SetParent(GameObject* newParent)
{
    if (parent == newParent)
    {
        return;
    }

    // Remove from old parent
    if (parent != nullptr)
    {
        parent->RemoveChild(this);
    }

    parent = newParent;

    // Add to new parent
    if (parent != nullptr)
    {
        parent->AddChild(this);
    }
}

// =========================================================
// GET PARENT
// =========================================================

GameObject* GameObject::GetParent() const
{
    return parent;
}

// =========================================================
// ADD CHILD
// =========================================================

void GameObject::AddChild(GameObject* child)
{
    if (child == nullptr)
    {
        return;
    }

    if (child == this)
    {
        return;
    }

    // Prevent duplicate children
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

// =========================================================
// REMOVE CHILD
// =========================================================

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

// =========================================================
// GET CHILDREN
// =========================================================

const std::vector<GameObject*>& GameObject::GetChildren() const
{
    return children;
}

// =========================================================
// HAS CHILDREN
// =========================================================

bool GameObject::HasChildren() const
{
    return !children.empty();
}