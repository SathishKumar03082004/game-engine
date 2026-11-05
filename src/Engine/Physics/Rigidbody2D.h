#pragma once

#include "../Core/Component.h"
#include "raylib.h"

class Rigidbody2D final : public Component
{
public:
    void SetMass(float value);
    float GetMass() const;
    void SetVelocity(Vector2 value);
    Vector2 GetVelocity() const;
    void SetAcceleration(Vector2 value);
    Vector2 GetAcceleration() const;
    void SetGravityEnabled(bool value);
    bool IsGravityEnabled() const;
    void SetKinematic(bool value);
    bool IsKinematic() const;

    void AddForce(Vector2 force);
    void AddAcceleration(Vector2 value);
    void Integrate(float deltaTime, Vector2 gravity);
    std::unique_ptr<Component> Clone() const override;

private:
    float mass = 1.0f;
    Vector2 velocity{0.0f, 0.0f};
    Vector2 acceleration{0.0f, 0.0f};
    Vector2 accumulatedAcceleration{0.0f, 0.0f};
    bool gravityEnabled = true;
    bool kinematic = false;
};
