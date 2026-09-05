#pragma once

#include "raylib.h"

#include "../../Engine/Scene/Scene.h"

class Hierarchy{
    public:
        Hierarchy(Scene& scene);

        void Initialize();
        void Update();
        void Draw();

    private:
        Scene& scene;

        Rectangle panal;

        float height;
        float width;

        int selectedIndex;
};