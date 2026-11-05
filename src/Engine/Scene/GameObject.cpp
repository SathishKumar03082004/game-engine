
#include "GameObject.h"
#include "../Animation/AnimationController.h"
#include "../Physics/BoxCollider2D.h"
#include "../Rendering/Renderer2D.h"
#include "../Rendering/ParticleEmitter.h"

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

// --------------------------------------------------
// Constructor
// --------------------------------------------------

GameObject::GameObject()
    : selected(false),
      size{100.0f, 100.0f},
      name("GameObject"),
      parent(nullptr)
{
    AddComponent<AnimationComponent>();
}

GameObject::~GameObject()
{
    for (const auto& component : components)
    {
        if (component != nullptr) component->Detach();
    }
}

// --------------------------------------------------
// Update
// --------------------------------------------------

void GameObject::Update()
{
    const float deltaTime = GetFrameTime();
    if (!componentsStarted)
    {
        componentsStarted = true;
        for (const auto& component : components)
        {
            if (component != nullptr) component->Start();
        }
    }

    for (const auto& component : components)
    {
        if (component != nullptr && component->IsEnabled()) component->Update(deltaTime);
    }

    for (GameObject* child : children)
    {
        if (child != nullptr)
        {
            child->Update();
        }
    }
}

// --------------------------------------------------
// Draw
// --------------------------------------------------

void GameObject::Draw()
{
    Renderer2D renderer;
    renderer.BeginFrame();
    SubmitRender(renderer);
    renderer.Flush();
}

void GameObject::SubmitRender(Renderer2D& renderer) const
{
    const Vector2 position = GetWorldPosition();
    const Vector2 worldScale = GetWorldScale();
    const float rotation = GetWorldRotation();
    const AnimationComponent* animation = GetComponent<AnimationComponent>();
    const bool drawAnimation = animation != nullptr && animation->IsEnabled() && animation->IsLoaded();
    const Texture2D* animatedTexture = drawAnimation ? animation->GetActiveTexture() : nullptr;

    const float baseWidth = drawAnimation ? static_cast<float>(animation->GetFrameWidth()) :
        (spriteRenderer.IsLoaded() ? static_cast<float>(spriteRenderer.GetWidth()) : size.x);
    const float baseHeight = drawAnimation ? static_cast<float>(animation->GetFrameHeight()) :
        (spriteRenderer.IsLoaded() ? static_cast<float>(spriteRenderer.GetHeight()) : size.y);
    const float width = baseWidth * worldScale.x;
    const float height = baseHeight * worldScale.y;
    const Rectangle destination{position.x, position.y, width, height};
    const Vector2 origin{width * 0.5f, height * 0.5f};

    if (drawAnimation && animatedTexture != nullptr)
        renderer.SubmitSprite(const_cast<Texture2D*>(animatedTexture), animation->GetActiveSourceRect(worldScale),
            destination, origin, rotation, WHITE);
    else if (spriteRenderer.IsLoaded())
    {
        Texture2D* texture = spriteRenderer.GetTexture();
        renderer.SubmitSprite(texture, {0.0f, 0.0f, static_cast<float>(texture->width), static_cast<float>(texture->height)},
            destination, origin, rotation, spriteRenderer.GetTint());
    }
    else
        renderer.SubmitRectangle({position.x, position.y, std::fabs(width), std::fabs(height)},
            {std::fabs(width) * 0.5f, std::fabs(height) * 0.5f}, rotation, selected ? SKYBLUE : GRAY);

    const ParticleEmitter* particles = GetComponent<ParticleEmitter>();
    if (particles != nullptr && particles->IsEnabled()) particles->Submit(renderer, position);

    if (!selected) return;
    const Vector2 corners[4] = {
        {-std::fabs(width) * 0.5f, -std::fabs(height) * 0.5f},
        { std::fabs(width) * 0.5f, -std::fabs(height) * 0.5f},
        { std::fabs(width) * 0.5f,  std::fabs(height) * 0.5f},
        {-std::fabs(width) * 0.5f,  std::fabs(height) * 0.5f}
    };
    Vector2 worldCorners[4];
    for (int i = 0; i < 4; ++i)
    {
        const Vector2 rotated = RotateVector(corners[i], rotation);
        worldCorners[i] = {position.x + rotated.x, position.y + rotated.y};
    }
    for (int i = 0; i < 4; ++i)
        renderer.SubmitLine(worldCorners[i], worldCorners[(i + 1) % 4], 3.0f, BLUE);

    const BoxCollider2D* collider = GetComponent<BoxCollider2D>();
    if (collider != nullptr && collider->IsEnabled())
    {
        const Rectangle bounds = collider->GetWorldBounds();
        const Color color = collider->IsTrigger() ? ORANGE : LIME;
        const Vector2 points[4] = {{bounds.x, bounds.y}, {bounds.x + bounds.width, bounds.y},
            {bounds.x + bounds.width, bounds.y + bounds.height}, {bounds.x, bounds.y + bounds.height}};
        for (int i = 0; i < 4; ++i) renderer.SubmitLine(points[i], points[(i + 1) % 4], 2.0f, color);
    }
}
// Transform
// --------------------------------------------------

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

    return parent->TransformPoint(
        transform.GetPosition()
    );
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

// --------------------------------------------------
// Render Layer and Sorting
// --------------------------------------------------

void GameObject::SetRenderLayer(RenderLayer layer)
{
    renderLayer = layer;
}

RenderLayer GameObject::GetRenderLayer() const
{
    return renderLayer;
}

void GameObject::SetSortingOrder(int order)
{
    sortingOrder = order;
}

int GameObject::GetSortingOrder() const
{
    return sortingOrder;
}

// --------------------------------------------------
// Sprite Renderer
// --------------------------------------------------

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

bool GameObject::SetSprite(const AssetReference& reference)
{
    return spriteRenderer.LoadSprite(reference);
}

void GameObject::ClearSprite()
{
    spriteRenderer.UnloadSprite();
}

// --------------------------------------------------
// Animation Component - Day 25
// --------------------------------------------------

AnimationComponent& GameObject::GetAnimation()
{
    AnimationComponent* animation = GetComponent<AnimationComponent>();
    return animation != nullptr ? *animation : AddComponent<AnimationComponent>();
}

const AnimationComponent& GameObject::GetAnimation() const
{
    const AnimationComponent* animation = GetComponent<AnimationComponent>();
    if (animation != nullptr) return *animation;
    static const AnimationComponent emptyAnimation;
    return emptyAnimation;
}

const std::vector<std::unique_ptr<Component>>& GameObject::GetComponents() const
{
    return components;
}

// --------------------------------------------------
// Selection
// --------------------------------------------------

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

    const AnimationComponent* animation = GetComponent<AnimationComponent>();
    const float halfWidth = animation != nullptr && animation->IsEnabled() && animation->IsLoaded()
        ? static_cast<float>(animation->GetFrameWidth()) * 0.5f
        : (spriteRenderer.IsLoaded()
            ? static_cast<float>(spriteRenderer.GetWidth()) * 0.5f
            : size.x * 0.5f);

    const float halfHeight = animation != nullptr && animation->IsEnabled() && animation->IsLoaded()
        ? static_cast<float>(animation->GetFrameHeight()) * 0.5f
        : (spriteRenderer.IsLoaded()
            ? static_cast<float>(spriteRenderer.GetHeight()) * 0.5f
            : size.y * 0.5f);

    return localPoint.x >= -halfWidth &&
           localPoint.x <= halfWidth &&
           localPoint.y >= -halfHeight &&
           localPoint.y <= halfHeight;
}

// --------------------------------------------------
// Name
// --------------------------------------------------

void GameObject::SetName(const std::string& newName)
{
    name = newName;
}

const std::string& GameObject::GetName() const
{
    return name;
}

// --------------------------------------------------
// Parent and Child Hierarchy
// --------------------------------------------------

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
