#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"

class Hierarchy
{
public:

    Hierarchy(Scene& scene);

    void Initialize();
    void Update();
    void Draw();

    bool IsMouseOver() const;

private:

    Scene& scene;

    Rectangle panel;

    float width;
    float height;

    int selectedIndex;
};