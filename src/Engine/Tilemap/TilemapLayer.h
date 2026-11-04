#pragma once

#include "Tile.h"

#include <string>
#include <unordered_map>

class TilemapLayer
{
public:
    TilemapLayer() = default;
    TilemapLayer(std::string name, int drawOrder);
    const std::string& GetName() const;
    void SetName(const std::string& name);
    int GetDrawOrder() const;
    void SetDrawOrder(int order);
    void SetTile(TileCoordinate coordinate, Tile tile);
    Tile GetTile(TileCoordinate coordinate) const;
    bool RemoveTile(TileCoordinate coordinate);
    void Clear();
    const std::unordered_map<TileCoordinate, Tile, TileCoordinateHash>& GetTiles() const;

private:
    std::string name = "Layer";
    int drawOrder = 0;
    std::unordered_map<TileCoordinate, Tile, TileCoordinateHash> tiles;
};
