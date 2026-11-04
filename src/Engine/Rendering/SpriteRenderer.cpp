#include "SpriteRenderer.h"
#include "../Resources/ResourceManager.h"

SpriteRenderer::~SpriteRenderer() { UnloadSprite(); }

bool SpriteRenderer::LoadSprite(const std::string& path)
{
    return LoadSprite(AssetReference::FromPath(path, AssetType::TextureAsset));
}

bool SpriteRenderer::LoadSprite(const AssetReference& reference)
{
    if (!reference.IsValid()) return false;
    if (reference.path == assetReference.path && texture != nullptr) return true;

    Texture2D* newTexture = ResourceManager::GetInstance().LoadTextureAsset(reference);
    if (newTexture == nullptr) return false;

    UnloadSprite();
    texture = newTexture;
    assetReference = reference;
    return true;
}

void SpriteRenderer::UnloadSprite()
{
    if (texture != nullptr && assetReference.IsValid())
        ResourceManager::GetInstance().ReleaseTexture(assetReference);
    texture = nullptr;
    assetReference = {};
}

bool SpriteRenderer::IsLoaded() const { return texture != nullptr; }
void SpriteRenderer::SetTint(Color newTint) { tint = newTint; }
Color SpriteRenderer::GetTint() const { return tint; }
int SpriteRenderer::GetWidth() const { return texture != nullptr ? texture->width : 0; }
int SpriteRenderer::GetHeight() const { return texture != nullptr ? texture->height : 0; }
const std::string& SpriteRenderer::GetFilePath() const { return assetReference.path; }
const AssetReference& SpriteRenderer::GetAssetReference() const { return assetReference; }

void SpriteRenderer::Draw(Vector2 position, Vector2 scale, float rotation) const
{
    if (texture == nullptr) return;
    Rectangle source{0.0f, 0.0f, static_cast<float>(texture->width), static_cast<float>(texture->height)};
    if (scale.x < 0.0f) source.width *= -1.0f;
    if (scale.y < 0.0f) source.height *= -1.0f;
    const float width = static_cast<float>(texture->width) * scale.x;
    const float height = static_cast<float>(texture->height) * scale.y;
    const Rectangle destination{position.x, position.y, width, height};
    const Vector2 origin{width * 0.5f, height * 0.5f};
    DrawTexturePro(*texture, source, destination, origin, rotation, tint);
}
