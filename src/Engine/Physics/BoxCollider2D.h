#pragma once

#include "../Core/Component.h"
#include "raylib.h"

class BoxCollider2D final : public Component
{
public:
    BoxCollider2D() = default;
    explicit BoxCollider2D(Vector2 size, Vector2 offset = {0.0f, 0.0f});

    void SetSize(Vector2 newSize);
    Vector2 GetSize() const;
    void SetOffset(Vector2 newOffset);
    Vector2 GetOffset() const;
    void SetTrigger(bool value);
    bool IsTrigger() const;
    std::unique_ptr<Component> Clone() const override;

    // Axis-aligned world bounds enclosing the transformed local box.
    Rectangle GetWorldBounds() const;
    bool Overlaps(const BoxCollider2D& other) const;

private:
    Vector2 size{100.0f, 100.0f};
    Vector2 offset{0.0f, 0.0f};
    bool trigger = false;
};
