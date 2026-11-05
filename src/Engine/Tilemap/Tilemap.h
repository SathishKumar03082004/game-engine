#pragma once

#include "TilemapLayer.h"
#include "Tileset.h"

#include <string>
#include <vector>

struct SolidTileBounds
{
    int x = 0;
    int y = 0;
    Rectangle bounds{};
};

class Tilemap
{
public:
    Tilemap();
    Tilemap(const Tilemap&) = delete;
    Tilemap& operator=(const Tilemap&) = delete;

    bool SetTileset(const std::string& path, int tileSize);
    bool SetTileset(const AssetReference& reference, int tileSize);
    Tileset& GetTileset();
    const Tileset& GetTileset() const;
    int GetTileSize() const;
    void SetTileSize(int size);
    TileCoordinate WorldToTile(Vector2 worldPosition) const;
    Vector2 TileToWorld(TileCoordinate coordinate) const;
    TilemapLayer* GetLayer(const std::string& name);
    const TilemapLayer* GetLayer(const std::string& name) const;
    TilemapLayer* GetLayer(std::size_t index);
    const TilemapLayer* GetLayer(std::size_t index) const;
    TilemapLayer& AddLayer(const std::string& name, int drawOrder);
    const std::vector<TilemapLayer>& GetLayers() const;
    void ResetDefaultLayers();
    void Clear();
    void Draw() const;
    void DrawLayer(const std::string& name) const;
    std::vector<SolidTileBounds> GetSolidTileBounds() const;
    bool Save(const std::string& path) const;
    bool Load(const std::string& path);
    bool CloneTo(Tilemap& destination) const;

private:
    Tileset tileset;
    int tileSize = 32;
    std::vector<TilemapLayer> layers;
};
