#include "TilemapLayer.h"

#include <utility>

TilemapLayer::TilemapLayer(std::string layerName, int order)
    : name(std::move(layerName)), drawOrder(order) {}
const std::string& TilemapLayer::GetName() const { return name; }
void TilemapLayer::SetName(const std::string& value) { if (!value.empty()) name = value; }
int TilemapLayer::GetDrawOrder() const { return drawOrder; }
void TilemapLayer::SetDrawOrder(int value) { drawOrder = value; }
void TilemapLayer::SetTile(TileCoordinate coordinate, Tile tile)
{
    if (tile.IsEmpty()) tiles.erase(coordinate);
    else tiles[coordinate] = tile;
}
Tile TilemapLayer::GetTile(TileCoordinate coordinate) const
{
    const auto found = tiles.find(coordinate);
    return found == tiles.end() ? Tile{} : found->second;
}
bool TilemapLayer::RemoveTile(TileCoordinate coordinate) { return tiles.erase(coordinate) != 0; }
void TilemapLayer::Clear() { tiles.clear(); }
const std::unordered_map<TileCoordinate, Tile, TileCoordinateHash>& TilemapLayer::GetTiles() const { return tiles; }
