#pragma once

#include "raylib.h"
#include "../Resources/AssetReference.h"

class Tileset
{
public:
    ~Tileset();
    bool Load(const AssetReference& reference, int tileSize);
    bool Load(const std::string& path, int tileSize);
    void Unload();
    void Swap(Tileset& other);
    bool IsLoaded() const;
    int GetTileSize() const;
    int GetColumns() const;
    int GetRows() const;
    int GetTileCount() const;
    const std::string& GetPath() const;
    const AssetReference& GetAssetReference() const;
    Rectangle GetSourceRect(int tileId) const;
    Texture2D* GetTexture() const;
    void DrawTile(int tileId, Rectangle destination, Color tint = WHITE) const;

private:
    AssetReference assetReference;
    Texture2D* texture = nullptr;
    int tileSize = 32;
    int columns = 0;
    int rows = 0;
};
