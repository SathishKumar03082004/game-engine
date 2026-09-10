#pragma once

#include "../Camera/Camera2D.h"
#include "../Scene/Scene.h"

#include "../../Editor/Hierarchy/Hierarchy.h"
#include "../../Editor/Inspector/Inspector.h"


class Engine
{
public:

    Engine();

    void Initialize();
    void Run();
    void Shutdown();


private:

    Camera2DSystem camera;

    Scene scene;

    Hierarchy* hierarchy;

    Inspector* inspector;
};