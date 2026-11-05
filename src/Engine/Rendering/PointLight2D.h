#pragma once
#include "../Core/Component.h"
#include <memory>

class PointLight2D : public Component
{
public:
    float radius = 240.0f;
    float intensity = 1.0f;
    Color color = WHITE;
    std::unique_ptr<Component> Clone() const override
    {
        auto clone = std::make_unique<PointLight2D>();
        clone->radius = radius; clone->intensity = intensity; clone->color = color;
        return clone;
    }
};
