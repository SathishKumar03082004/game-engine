#pragma once
#include "../Scene/Scene.h"

class UIManager
{
public:
    void Update(Scene& scene);
    void Draw(Scene& scene);
    void ReleaseResources(Scene& scene);
};
