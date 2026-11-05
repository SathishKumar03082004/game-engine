#include "ParticleEmitter.h"
#include "Renderer2D.h"
#include "../Scene/GameObject.h"
#include <algorithm>
#include <cmath>

void ParticleEmitter::Update(float dt)
{
    if (!IsEnabled() || !std::isfinite(dt) || dt <= 0.0f) return;
    for (auto& p : particles) { p.age += dt; p.position.x += p.velocity.x * dt; p.position.y += p.velocity.y * dt; }
    particles.erase(std::remove_if(particles.begin(), particles.end(), [this](const Particle& p) { return p.age >= lifetime; }), particles.end());
    if (lifetime <= 0.0f || emissionRate <= 0.0f || GetOwner() == nullptr) return;
    emissionAccumulator += dt * emissionRate;
    const int count = std::min(128, static_cast<int>(emissionAccumulator)); emissionAccumulator -= count;
    const Vector2 origin = GetOwner()->GetWorldPosition();
    for (int i = 0; i < count; ++i) { const float angle = GetRandomValue(0, 359) * DEG2RAD; particles.push_back({origin, {cosf(angle) * speed, sinf(angle) * speed}, 0.0f}); }
}
void ParticleEmitter::Submit(Renderer2D& renderer, Vector2) const
{
    for (const auto& p : particles) { Color tint = color; tint.a = static_cast<unsigned char>(color.a * std::clamp(1.0f - p.age / std::max(lifetime, 0.001f), 0.0f, 1.0f)); renderer.SubmitCircle(p.position, particleSize, tint); }
}
