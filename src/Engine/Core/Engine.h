#pragma once

#include "../Camera/Camera2D.h"
#include "../Scene/Scene.h"

class Engine
{
public:
    void Initialize();
    void Run();
    void Shutdown();

private:
    Camera2DSystem camera;
    Scene scene;
};