
#pragma once

#include "raylib.h"

#include "../Math/Transform.h"
#include "../Rendering/SpriteRenderer.h"
#include "../Rendering/RenderLayer.h"
#include "../Animation/AnimationComponent.h"

#include <string>
#include <vector>

class GameObject
{
public:
    GameObject();
    ~GameObject() = default;

    // Update and Draw
    void Update();
    void Draw();

    // Transform
    TransformComponent& GetTransform();

    Vector2 GetWorldPosition() const;
    float GetWorldRotation() const;
    Vector2 GetWorldScale() const;

    Vector2 TransformPoint(Vector2 localPoint) const;
    Vector2 InverseTransformPoint(Vector2 worldPoint) const;

    // Selection
    void SetSelected(bool selected);
    bool IsSelected() const;
    bool ContainsPoint(Vector2 worldPoint) const;

    // Name
    void SetName(const std::string& newName);
    const std::string& GetName() const;

    // Parent and Child
    void SetParent(GameObject* newParent);
    GameObject* GetParent() const;

    void AddChild(GameObject* child);
    void RemoveChild(GameObject* child);

    const std::vector<GameObject*>& GetChildren() const;
    bool HasChildren() const;

    // Sprite functionality
    SpriteRenderer& GetSpriteRenderer();
    const SpriteRenderer& GetSpriteRenderer() const;

    bool SetSprite(const std::string& path);
    void ClearSprite();

    // Render Layer and Sorting
    void SetRenderLayer(RenderLayer layer);
    RenderLayer GetRenderLayer() const;

    void SetSortingOrder(int order);
    int GetSortingOrder() const;

    // Day 25: Animation
    AnimationComponent& GetAnimation();
    const AnimationComponent& GetAnimation() const;

private:
    TransformComponent transform;

    bool selected;

    Vector2 size;

    std::string name;

    GameObject* parent;
    std::vector<GameObject*> children;

    SpriteRenderer spriteRenderer;

    AnimationComponent animation;

    // Existing render settings
    RenderLayer renderLayer = RenderLayer::Default;
    int sortingOrder = 0;
};
