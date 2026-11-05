#include "BoxCollider2D.h"
#include "../Scene/GameObject.h"

#include <algorithm>
#include <cmath>
#include <memory>

BoxCollider2D::BoxCollider2D(Vector2 newSize, Vector2 newOffset)
    : offset(newOffset)
{
    SetSize(newSize);
}

void BoxCollider2D::SetSize(Vector2 newSize)
{
    size = {std::isfinite(newSize.x) ? std::fabs(newSize.x) : 0.0f,
            std::isfinite(newSize.y) ? std::fabs(newSize.y) : 0.0f};
}

Vector2 BoxCollider2D::GetSize() const { return size; }
void BoxCollider2D::SetOffset(Vector2 newOffset)
{
    if (std::isfinite(newOffset.x) && std::isfinite(newOffset.y)) offset = newOffset;
}
Vector2 BoxCollider2D::GetOffset() const { return offset; }
void BoxCollider2D::SetTrigger(bool value) { trigger = value; }
bool BoxCollider2D::IsTrigger() const { return trigger; }
std::unique_ptr<Component> BoxCollider2D::Clone() const
{
    auto clone = std::make_unique<BoxCollider2D>(size, offset);
    clone->SetTrigger(trigger);
    return clone;
}

Rectangle BoxCollider2D::GetWorldBounds() const
{
    const GameObject* gameObject = GetOwner();
    if (gameObject == nullptr)
        return {offset.x - size.x * 0.5f, offset.y - size.y * 0.5f, size.x, size.y};

    const Vector2 half = {size.x * 0.5f, size.y * 0.5f};
    const Vector2 corners[4] = {
        {offset.x - half.x, offset.y - half.y},
        {offset.x + half.x, offset.y - half.y},
        {offset.x + half.x, offset.y + half.y},
        {offset.x - half.x, offset.y + half.y}
    };
    Vector2 first = gameObject->TransformPoint(corners[0]);
    float minX = first.x, maxX = first.x, minY = first.y, maxY = first.y;
    for (int index = 1; index < 4; ++index)
    {
        const Vector2 point = gameObject->TransformPoint(corners[index]);
        minX = std::min(minX, point.x);
        maxX = std::max(maxX, point.x);
        minY = std::min(minY, point.y);
        maxY = std::max(maxY, point.y);
    }
    return {minX, minY, maxX - minX, maxY - minY};
}

bool BoxCollider2D::Overlaps(const BoxCollider2D& other) const
{
    if (!IsEnabled() || !other.IsEnabled()) return false;
    const Rectangle first = GetWorldBounds();
    const Rectangle second = other.GetWorldBounds();
    return first.x < second.x + second.width &&
           first.x + first.width > second.x &&
           first.y < second.y + second.height &&
           first.y + first.height > second.y;
}
