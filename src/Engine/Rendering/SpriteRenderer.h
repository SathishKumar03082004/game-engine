#pragma once

#include "raylib.h"
#include "../Resources/AssetReference.h"

#include <string>

class SpriteRenderer
{
public:
    SpriteRenderer() = default;
    ~SpriteRenderer();

    SpriteRenderer(const SpriteRenderer&) = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;

    bool LoadSprite(const std::string& path);
    bool LoadSprite(const AssetReference& reference);

    void UnloadSprite();

    bool IsLoaded() const;

    void SetTint(Color newTint);
    Color GetTint() const;

    int GetWidth() const;
    int GetHeight() const;

    const std::string& GetFilePath() const;
    const AssetReference& GetAssetReference() const;

    void Draw(
        Vector2 position,
        Vector2 scale,
        float rotation
    ) const;

private:
    Texture2D* texture = nullptr;

    Color tint = WHITE;

    AssetReference assetReference;
};
