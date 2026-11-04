#include "Rigidbody2D.h"
#include "../Scene/GameObject.h"

#include <algorithm>
#include <cmath>

void Rigidbody2D::SetMass(float value)
{
    if (std::isfinite(value) && value > 0.0f) mass = value;
}
float Rigidbody2D::GetMass() const { return mass; }
void Rigidbody2D::SetVelocity(Vector2 value)
{
    if (std::isfinite(value.x) && std::isfinite(value.y)) velocity = value;
}
Vector2 Rigidbody2D::GetVelocity() const { return velocity; }
void Rigidbody2D::SetAcceleration(Vector2 value)
{
    if (std::isfinite(value.x) && std::isfinite(value.y)) acceleration = value;
}
Vector2 Rigidbody2D::GetAcceleration() const { return acceleration; }
void Rigidbody2D::SetGravityEnabled(bool value) { gravityEnabled = value; }
bool Rigidbody2D::IsGravityEnabled() const { return gravityEnabled; }
void Rigidbody2D::SetKinematic(bool value) { kinematic = value; }
bool Rigidbody2D::IsKinematic() const { return kinematic; }

void Rigidbody2D::AddForce(Vector2 force)
{
    if (!std::isfinite(force.x) || !std::isfinite(force.y)) return;
    accumulatedAcceleration.x += force.x / mass;
    accumulatedAcceleration.y += force.y / mass;
}

void Rigidbody2D::AddAcceleration(Vector2 value)
{
    if (!std::isfinite(value.x) || !std::isfinite(value.y)) return;
    accumulatedAcceleration.x += value.x;
    accumulatedAcceleration.y += value.y;
}

void Rigidbody2D::Integrate(float deltaTime, Vector2 gravity)
{
    if (!IsEnabled() || kinematic || GetOwner() == nullptr ||
        !std::isfinite(deltaTime) || deltaTime <= 0.0f) return;
    const Vector2 effectiveGravity = gravityEnabled ? gravity : Vector2{0.0f, 0.0f};
    velocity.x += (acceleration.x + accumulatedAcceleration.x + effectiveGravity.x) * deltaTime;
    velocity.y += (acceleration.y + accumulatedAcceleration.y + effectiveGravity.y) * deltaTime;
    GetOwner()->GetTransform().Translate({velocity.x * deltaTime, velocity.y * deltaTime});
    accumulatedAcceleration = {0.0f, 0.0f};
}
