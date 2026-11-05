#pragma once
#include "../Core/Component.h"

class PointLight2D : public Component
{
public:
    float radius = 240.0f;
    float intensity = 1.0f;
    Color color = WHITE;
};
