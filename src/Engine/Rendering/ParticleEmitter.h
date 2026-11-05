#pragma once
#include "raylib.h"
#include "../Core/Component.h"
#include <vector>

class Renderer2D;
class ParticleEmitter : public Component
{
public:
    float emissionRate = 12.0f;
    float lifetime = 1.2f;
    float speed = 90.0f;
    float particleSize = 4.0f;
    Color color = ORANGE;
    void Update(float deltaTime) override;
    void Submit(Renderer2D& renderer, Vector2 position) const;
    std::unique_ptr<Component> Clone() const override;
private:
    struct Particle { Vector2 position; Vector2 velocity; float age; };
    std::vector<Particle> particles;
    float emissionAccumulator = 0.0f;
};
