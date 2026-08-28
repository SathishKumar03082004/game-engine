#pragma once

#include "raylib.h"

class Scene{
    public:
        void Initialize();
        void Update();
        void Draw();

    private:
        void DrawGrid();

    private:
        int gridSize = 64;
        int gridExtent = 15000;
};