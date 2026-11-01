#include "SpriteRenderer.h"
#include "TextureManager.h"

SpriteRenderer::~SpriteRenderer()
{
    UnloadSprite();
}

bool SpriteRenderer::LoadSprite(
    const std::string& path
)
{
    if (path.empty())
    {
        return false;
    }

    if (path == filePath && texture != nullptr)
    {
        return true;
    }

    Texture2D* newTexture =
        TextureManager::GetInstance().LoadTextureAsset(path);

    if (newTexture == nullptr)
    {
        return false;
    }

    // Release the old asset only after the new one loads.
    UnloadSprite();

    texture = newTexture;
    filePath = path;

    return true;
}

void SpriteRenderer::UnloadSprite()
{
    if (texture != nullptr && !filePath.empty())
    {
        TextureManager::GetInstance().ReleaseTexture(filePath);
    }

    texture = nullptr;
    filePath.clear();
}

bool SpriteRenderer::IsLoaded() const
{
    return texture != nullptr;
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
    return texture != nullptr ? texture->width : 0;
}

int SpriteRenderer::GetHeight() const
{
    return texture != nullptr ? texture->height : 0;
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
    if (texture == nullptr)
    {
        return;
    }

    Rectangle source =
    {
        0.0f,
        0.0f,
        static_cast<float>(texture->width),
        static_cast<float>(texture->height)
    };

    // Support negative scale for flipping.
    if (scale.x < 0.0f)
    {
        source.width *= -1.0f;
    }

    if (scale.y < 0.0f)
    {
        source.height *= -1.0f;
    }

    float width =
        static_cast<float>(texture->width) * scale.x;

    float height =
        static_cast<float>(texture->height) * scale.y;

    Rectangle destination =
    {
        position.x,
        position.y,
        width,
        height
    };

    Vector2 origin =
    {
        width * 0.5f,
        height * 0.5f
    };

    DrawTexturePro(
        *texture,
        source,
        destination,
        origin,
        rotation,
        tint
    );
}