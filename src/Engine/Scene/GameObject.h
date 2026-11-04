
#pragma once

#include "raylib.h"

#include "../Math/Transform.h"
#include "../Rendering/SpriteRenderer.h"
#include "../Rendering/RenderLayer.h"
#include "../Animation/AnimationComponent.h"
#include "../Core/Component.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

class GameObject
{
public:
    GameObject();
    ~GameObject();

    template <typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* result = component.get();
        components.push_back(std::move(component));
        result->Attach(this);
        if (componentsStarted) result->Start();
        return *result;
    }

    template <typename T>
    T* GetComponent()
    {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        for (const auto& component : components)
        {
            if (T* result = dynamic_cast<T*>(component.get())) return result;
        }
        return nullptr;
    }

    template <typename T>
    const T* GetComponent() const
    {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        for (const auto& component : components)
        {
            if (const T* result = dynamic_cast<const T*>(component.get())) return result;
        }
        return nullptr;
    }

    template <typename T>
    bool RemoveComponent()
    {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        for (auto iterator = components.begin(); iterator != components.end(); ++iterator)
        {
            if (dynamic_cast<T*>(iterator->get()) != nullptr)
            {
                (*iterator)->Detach();
                components.erase(iterator);
                return true;
            }
        }
        return false;
    }

    const std::vector<std::unique_ptr<Component>>& GetComponents() const;

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

    std::vector<std::unique_ptr<Component>> components;
    bool componentsStarted = false;

    // Existing render settings
    RenderLayer renderLayer = RenderLayer::Default;
    int sortingOrder = 0;
};
