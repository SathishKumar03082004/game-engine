#include "Tileset.h"
#include "../Resources/ResourceManager.h"
#include <utility>

Tileset::~Tileset() { Unload(); }
bool Tileset::Load(const std::string& path, int size)
{
    return Load(AssetReference::FromPath(path, AssetType::TextureAsset), size);
}
bool Tileset::Load(const AssetReference& reference, int size)
{
    if (size < 1 || !reference.IsValid()) return false;
    if (texture != nullptr && assetReference.path == reference.path && tileSize == size) return true;
    Texture2D* loaded = ResourceManager::GetInstance().LoadTextureAsset(reference);
    if (loaded == nullptr || loaded->width < size || loaded->height < size)
    {
        if (loaded != nullptr) ResourceManager::GetInstance().ReleaseTexture(reference);
        return false;
    }
    Unload();
    texture = loaded;
    assetReference = reference;
    tileSize = size;
    columns = texture->width / tileSize;
    rows = texture->height / tileSize;
    return columns > 0 && rows > 0;
}
void Tileset::Unload()
{
    if (texture != nullptr) ResourceManager::GetInstance().ReleaseTexture(assetReference);
    texture = nullptr;
    assetReference = {};
    columns = rows = 0;
}
void Tileset::Swap(Tileset& other)
{
    using std::swap;
    swap(assetReference, other.assetReference);
    swap(texture, other.texture);
    swap(tileSize, other.tileSize);
    swap(columns, other.columns);
    swap(rows, other.rows);
}
bool Tileset::IsLoaded() const { return texture != nullptr && columns > 0 && rows > 0; }
int Tileset::GetTileSize() const { return tileSize; }
int Tileset::GetColumns() const { return columns; }
int Tileset::GetRows() const { return rows; }
int Tileset::GetTileCount() const { return columns * rows; }
const std::string& Tileset::GetPath() const { return assetReference.path; }
const AssetReference& Tileset::GetAssetReference() const { return assetReference; }
Texture2D* Tileset::GetTexture() const { return texture; }
Rectangle Tileset::GetSourceRect(int tileId) const
{
    if (!IsLoaded() || tileId < 0 || tileId >= GetTileCount()) return {};
    return {static_cast<float>((tileId % columns) * tileSize),
            static_cast<float>((tileId / columns) * tileSize),
            static_cast<float>(tileSize), static_cast<float>(tileSize)};
}
void Tileset::DrawTile(int tileId, Rectangle destination, Color tint) const
{
    if (texture == nullptr) return;
    const Rectangle source = GetSourceRect(tileId);
    if (source.width > 0.0f) DrawTexturePro(*texture, source, destination, {0.0f, 0.0f}, 0.0f, tint);
}
