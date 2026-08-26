#pragma once

#include "raylib.h"

class Camera2DSystem{
    public:
        void Initialize();
        void Update();
        void Begin();
        void End();
        Camera2D GetCamera();

    private:
        Camera2D camera;
};