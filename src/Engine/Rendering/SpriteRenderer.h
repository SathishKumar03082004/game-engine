#pragma once

#include "raylib.h"

#include <string>

class SpriteRenderer
{
public:
    SpriteRenderer();
    ~SpriteRenderer();

    // A renderer owns its texture and must not be copied.
    SpriteRenderer(const SpriteRenderer&) = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;

    bool LoadSprite(const std::string& filePath);
    void UnloadSprite();

    bool IsLoaded() const;

    void SetTint(Color newTint);
    Color GetTint() const;

    int GetWidth() const;
    int GetHeight() const;

    const std::string& GetFilePath() const;

    void Draw(
        Vector2 position,
        Vector2 scale,
        float rotation
    ) const;

private:
    Texture2D texture{};
    Color tint = WHITE;
    std::string filePath;
    bool loaded = false;
};