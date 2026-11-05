#include "ParallaxBackground.h"
#include <cmath>
ParallaxBackground::~ParallaxBackground() { Clear(); }
bool ParallaxBackground::AddLayer(const std::string& path, float factor, float speed, Color tint)
{
    Texture2D texture = LoadTexture(path.c_str()); if (texture.id == 0) return false;
    layers.push_back({texture, factor, speed, 0.0f, tint}); return true;
}
void ParallaxBackground::Draw(Vector2 cameraOffset, int width, int height)
{
    for (auto& layer : layers) { layer.offset += layer.speed * GetFrameTime(); const float scale = static_cast<float>(height) / layer.texture.height;
        const float tileWidth = layer.texture.width * scale; if (tileWidth <= 0.0f) continue;
        float x = -std::fmod(cameraOffset.x * layer.factor + layer.offset, tileWidth); if (x > 0) x -= tileWidth;
        for (; x < width; x += tileWidth) DrawTexturePro(layer.texture, {0,0,(float)layer.texture.width,(float)layer.texture.height}, {x,0,tileWidth,(float)height}, {0,0},0,layer.tint);
    }
}
void ParallaxBackground::Clear() { for (auto& layer : layers) if (layer.texture.id) UnloadTexture(layer.texture); layers.clear(); }
