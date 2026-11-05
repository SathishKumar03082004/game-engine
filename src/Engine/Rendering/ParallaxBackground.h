#pragma once
#include "raylib.h"
#include <string>
#include <vector>

class ParallaxBackground
{
public:
    ~ParallaxBackground();
    bool AddLayer(const std::string& path, float factor, float speed = 0.0f, Color tint = WHITE);
    void Draw(Vector2 cameraOffset, int screenWidth, int screenHeight);
    void Clear();
private:
    struct Layer { Texture2D texture{}; float factor; float speed; float offset = 0.0f; Color tint; };
    std::vector<Layer> layers;
};
