#include "SpriteRenderer.h"

SpriteRenderer::SpriteRenderer()
    : texture{},
      tint(WHITE),
      filePath(),
      loaded(false)
{
}

SpriteRenderer::~SpriteRenderer()
{
    UnloadSprite();
}

bool SpriteRenderer::LoadSprite(const std::string& path)
{
    if (path.empty())
    {
        TraceLog(
            LOG_WARNING,
            "SpriteRenderer: Empty texture path."
        );

        return false;
    }

    if (!FileExists(path.c_str()))
    {
        TraceLog(
            LOG_WARNING,
            "SpriteRenderer: File not found: %s",
            path.c_str()
        );

        return false;
    }

    // Keep the existing texture if loading the replacement fails.
    Texture2D newTexture = LoadTexture(path.c_str());

    if (newTexture.id == 0)
    {
        TraceLog(
            LOG_WARNING,
            "SpriteRenderer: Failed to load texture: %s",
            path.c_str()
        );

        return false;
    }

    UnloadSprite();

    texture = newTexture;
    filePath = path;
    loaded = true;

    SetTextureFilter(
        texture,
        TEXTURE_FILTER_BILINEAR
    );

    TraceLog(
        LOG_INFO,
        "SpriteRenderer: Loaded texture: %s",
        path.c_str()
    );

    return true;
}

void SpriteRenderer::UnloadSprite()
{
    if (loaded)
    {
        UnloadTexture(texture);
    }

    texture = {};
    filePath.clear();
    loaded = false;
}

bool SpriteRenderer::IsLoaded() const
{
    return loaded;
}

void SpriteRenderer::SetTint(Color newTint)
{
    tint = newTint;
}

Color SpriteRenderer::GetTint() const
{
    return tint;
}

int SpriteRenderer::GetWidth() const
{
    return loaded ? texture.width : 0;
}

int SpriteRenderer::GetHeight() const
{
    return loaded ? texture.height : 0;
}

const std::string& SpriteRenderer::GetFilePath() const
{
    return filePath;
}

void SpriteRenderer::Draw(
    Vector2 position,
    Vector2 scale,
    float rotation
) const
{
    if (!loaded)
    {
        return;
    }

    if (scale.x == 0.0f || scale.y == 0.0f)
    {
        return;
    }

    // Negative scale flips the sprite.
    Rectangle source = {
        0.0f,
        0.0f,
        static_cast<float>(texture.width),
        static_cast<float>(texture.height)
    };

    if (scale.x < 0.0f)
    {
        source.width = -source.width;
    }

    if (scale.y < 0.0f)
    {
        source.height = -source.height;
    }

    Rectangle destination = {
        position.x,
        position.y,
        static_cast<float>(texture.width) * (scale.x < 0.0f ? -scale.x : scale.x),
        static_cast<float>(texture.height) * (scale.y < 0.0f ? -scale.y : scale.y)
    };

    // Rotate around the sprite's center.
    Vector2 origin = {
        destination.width * 0.5f,
        destination.height * 0.5f
    };

    DrawTexturePro(
        texture,
        source,
        destination,
        origin,
        rotation,
        tint
    );
}