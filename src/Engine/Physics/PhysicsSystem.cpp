#include "PhysicsSystem.h"

#include "BoxCollider2D.h"
#include "Rigidbody2D.h"
#include "../Scene/GameObject.h"
#include "../Scene/Scene.h"

#include <algorithm>
#include <cmath>

namespace
{
    float InverseMass(const Rigidbody2D* body)
    {
        return body == nullptr || !body->IsEnabled() || body->IsKinematic() || body->GetMass() <= 0.0f
            ? 0.0f : 1.0f / body->GetMass();
    }

    void TranslateWorld(GameObject* object, Vector2 amount)
    {
        if (object == nullptr) return;
        const Vector2 world = object->GetWorldPosition();
        const Vector2 movedWorld{world.x + amount.x, world.y + amount.y};
        GameObject* parent = object->GetParent();
        object->GetTransform().SetPosition(parent == nullptr ? movedWorld : parent->InverseTransformPoint(movedWorld));
    }

}

void PhysicsSystem::Update(Scene& scene)
{
    collisionPairs.clear();
    const float deltaTime = GetFrameTime();
    std::vector<BoxCollider2D*> colliders;

    // Integrate bodies before broad-phase/narrow-phase overlap checks.
    for (const auto& object : scene.GetGameObjects())
    {
        if (object == nullptr) continue;
        if (Rigidbody2D* body = object->GetComponent<Rigidbody2D>())
            body->Integrate(deltaTime, gravity);
        for (const auto& component : object->GetComponents())
        {
            if (component == nullptr || !component->IsEnabled()) continue;
            if (auto* collider = dynamic_cast<BoxCollider2D*>(component.get()))
                colliders.push_back(collider);
        }
    }

    RefreshCollisionPairs(colliders);
    // A few solver passes handle stacks more reliably than a single pair pass.
    for (int iteration = 0; iteration < 4; ++iteration) ResolveCollisions(colliders);
}

void PhysicsSystem::SetGravity(Vector2 value)
{
    if (std::isfinite(value.x) && std::isfinite(value.y)) gravity = value;
}

Vector2 PhysicsSystem::GetGravity() const { return gravity; }

void PhysicsSystem::ResolveCollisions(const std::vector<BoxCollider2D*>& colliders)
{
    for (std::size_t firstIndex = 0; firstIndex < colliders.size(); ++firstIndex)
    {
        BoxCollider2D* first = colliders[firstIndex];
        if (first == nullptr || !first->IsEnabled() || first->GetOwner() == nullptr) continue;
        for (std::size_t secondIndex = firstIndex + 1; secondIndex < colliders.size(); ++secondIndex)
        {
            BoxCollider2D* second = colliders[secondIndex];
            if (second == nullptr || !second->IsEnabled() || second->GetOwner() == nullptr ||
                first->GetOwner() == second->GetOwner() || first->IsTrigger() || second->IsTrigger()) continue;

            const Rectangle a = first->GetWorldBounds();
            const Rectangle b = second->GetWorldBounds();
            const float overlapX = std::min(a.x + a.width, b.x + b.width) - std::max(a.x, b.x);
            const float overlapY = std::min(a.y + a.height, b.y + b.height) - std::max(a.y, b.y);
            if (overlapX <= 0.0f || overlapY <= 0.0f) continue;

            Rigidbody2D* firstBody = first->GetOwner()->GetComponent<Rigidbody2D>();
            Rigidbody2D* secondBody = second->GetOwner()->GetComponent<Rigidbody2D>();
            const float inverseFirst = InverseMass(firstBody);
            const float inverseSecond = InverseMass(secondBody);
            const float inverseTotal = inverseFirst + inverseSecond;
            if (inverseTotal <= 0.0f) continue;

            const Vector2 centerA{a.x + a.width * 0.5f, a.y + a.height * 0.5f};
            const Vector2 centerB{b.x + b.width * 0.5f, b.y + b.height * 0.5f};
            Vector2 normal{};
            float penetration = 0.0f;
            if (overlapX < overlapY)
            {
                normal = {centerB.x >= centerA.x ? 1.0f : -1.0f, 0.0f};
                penetration = overlapX;
            }
            else
            {
                normal = {0.0f, centerB.y >= centerA.y ? 1.0f : -1.0f};
                penetration = overlapY;
            }

            TranslateWorld(first->GetOwner(), {-normal.x * penetration * inverseFirst / inverseTotal,
                                                  -normal.y * penetration * inverseFirst / inverseTotal});
            TranslateWorld(second->GetOwner(), {normal.x * penetration * inverseSecond / inverseTotal,
                                                   normal.y * penetration * inverseSecond / inverseTotal});

            // Inelastic response: remove only velocity directed into the contact.
            Vector2 velocityA = firstBody == nullptr ? Vector2{0.0f, 0.0f} : firstBody->GetVelocity();
            Vector2 velocityB = secondBody == nullptr ? Vector2{0.0f, 0.0f} : secondBody->GetVelocity();
            const float relativeNormalSpeed = (velocityB.x - velocityA.x) * normal.x +
                                              (velocityB.y - velocityA.y) * normal.y;
            if (relativeNormalSpeed < 0.0f)
            {
                const float impulse = -relativeNormalSpeed / inverseTotal;
                if (firstBody != nullptr && inverseFirst > 0.0f)
                    firstBody->SetVelocity({velocityA.x - impulse * inverseFirst * normal.x,
                                            velocityA.y - impulse * inverseFirst * normal.y});
                if (secondBody != nullptr && inverseSecond > 0.0f)
                    secondBody->SetVelocity({velocityB.x + impulse * inverseSecond * normal.x,
                                             velocityB.y + impulse * inverseSecond * normal.y});
            }
        }
    }
}

void PhysicsSystem::RefreshCollisionPairs(const std::vector<BoxCollider2D*>& colliders)
{
    for (std::size_t first = 0; first < colliders.size(); ++first)
    {
        for (std::size_t second = first + 1; second < colliders.size(); ++second)
        {
            if (colliders[first] == nullptr || colliders[second] == nullptr ||
                colliders[first]->GetOwner() == colliders[second]->GetOwner()) continue;
            if (colliders[first]->Overlaps(*colliders[second]))
                collisionPairs.push_back({colliders[first], colliders[second]});
        }
    }
}

const std::vector<CollisionPair>& PhysicsSystem::GetCollisionPairs() const { return collisionPairs; }

bool PhysicsSystem::AreOverlapping(const BoxCollider2D& first, const BoxCollider2D& second) const
{
    return first.GetOwner() != second.GetOwner() && first.Overlaps(second);
}

void PhysicsSystem::Clear() { collisionPairs.clear(); }
