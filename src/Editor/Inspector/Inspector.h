#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"

class Inspector
{
public:

    Inspector(Scene& scene);

    void Initialize();
    void Update();
    void Draw();

private:

    Scene& scene;

    Rectangle panel;

    float width;
    float height;

    void DrawTransform(GameObject* object);
};