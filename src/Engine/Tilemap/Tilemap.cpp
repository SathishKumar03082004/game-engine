#include "Tilemap.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <utility>

Tilemap::Tilemap() { ResetDefaultLayers(); }

bool Tilemap::SetTileset(const std::string& path, int size)
{
    if (!tileset.Load(path, size)) return false;
    tileSize = size;
    return true;
}
bool Tilemap::SetTileset(const AssetReference& reference, int size)
{
    if (!tileset.Load(reference, size)) return false;
    tileSize = size;
    return true;
}
Tileset& Tilemap::GetTileset() { return tileset; }
const Tileset& Tilemap::GetTileset() const { return tileset; }
int Tilemap::GetTileSize() const { return tileSize; }
void Tilemap::SetTileSize(int size) { if (size >= 8 && size <= 256) tileSize = size; }

TileCoordinate Tilemap::WorldToTile(Vector2 position) const
{
    const float size = static_cast<float>(std::max(1, GetTileSize()));
    return {static_cast<int>(std::floor(position.x / size)), static_cast<int>(std::floor(position.y / size))};
}
Vector2 Tilemap::TileToWorld(TileCoordinate coordinate) const
{
    const float size = static_cast<float>(GetTileSize());
    return {static_cast<float>(coordinate.x) * size, static_cast<float>(coordinate.y) * size};
}
TilemapLayer* Tilemap::GetLayer(const std::string& name)
{
    for (TilemapLayer& layer : layers) if (layer.GetName() == name) return &layer;
    return nullptr;
}
const TilemapLayer* Tilemap::GetLayer(const std::string& name) const
{
    for (const TilemapLayer& layer : layers) if (layer.GetName() == name) return &layer;
    return nullptr;
}
TilemapLayer* Tilemap::GetLayer(std::size_t index) { return index < layers.size() ? &layers[index] : nullptr; }
const TilemapLayer* Tilemap::GetLayer(std::size_t index) const { return index < layers.size() ? &layers[index] : nullptr; }
TilemapLayer& Tilemap::AddLayer(const std::string& name, int drawOrder)
{
    if (TilemapLayer* existing = GetLayer(name)) return *existing;
    layers.emplace_back(name, drawOrder);
    return layers.back();
}
const std::vector<TilemapLayer>& Tilemap::GetLayers() const { return layers; }
void Tilemap::ResetDefaultLayers()
{
    layers.clear();
    layers.emplace_back("Background", 0);
    layers.emplace_back("Ground", 1);
    layers.emplace_back("Objects", 2);
    layers.emplace_back("Foreground", 3);
}
void Tilemap::Clear()
{
    tileset.Unload();
    tileSize = 32;
    ResetDefaultLayers();
}

void Tilemap::Draw() const
{
    if (!tileset.IsLoaded()) return;
    std::vector<const TilemapLayer*> ordered;
    for (const TilemapLayer& layer : layers) ordered.push_back(&layer);
    std::stable_sort(ordered.begin(), ordered.end(), [](const TilemapLayer* a, const TilemapLayer* b)
        { return a->GetDrawOrder() < b->GetDrawOrder(); });
    const float size = static_cast<float>(GetTileSize());
    for (const TilemapLayer* layer : ordered)
    {
        for (const auto& entry : layer->GetTiles())
        {
            const Vector2 world = TileToWorld(entry.first);
            tileset.DrawTile(entry.second.id, {world.x, world.y, size, size});
        }
    }
}

void Tilemap::DrawLayer(const std::string& name) const
{
    const TilemapLayer* layer = GetLayer(name);
    if (layer == nullptr || !tileset.IsLoaded()) return;
    const float size = static_cast<float>(GetTileSize());
    for (const auto& entry : layer->GetTiles())
    {
        const Vector2 world = TileToWorld(entry.first);
        tileset.DrawTile(entry.second.id, {world.x, world.y, size, size});
    }
}

std::vector<SolidTileBounds> Tilemap::GetSolidTileBounds() const
{
    std::vector<SolidTileBounds> result;
    const float size = static_cast<float>(GetTileSize());
    if (size <= 0.0f) return result;
    for (const TilemapLayer& layer : layers)
        for (const auto& entry : layer.GetTiles())
            if (entry.second.collidable)
            {
                const Vector2 world = TileToWorld(entry.first);
                result.push_back({entry.first.x, entry.first.y, {world.x, world.y, size, size}});
            }
    return result;
}

bool Tilemap::Save(const std::string& path) const
{
    if (path.empty()) return false;
    std::error_code error;
    const std::filesystem::path filePath(path);
    if (!filePath.parent_path().empty())
    {
        std::filesystem::create_directories(filePath.parent_path(), error);
        if (error) return false;
    }
    std::ofstream file(filePath, std::ios::trunc);
    if (!file) return false;
    file << "RAYLIB_TILEMAP 1 " << std::quoted(tileset.GetPath()) << ' ' << tileSize << ' ' << layers.size() << '\n';
    for (const TilemapLayer& layer : layers)
    {
        file << "LAYER " << layer.GetDrawOrder() << ' ' << std::quoted(layer.GetName()) << ' '
             << layer.GetTiles().size() << '\n';
        std::vector<std::pair<TileCoordinate, Tile>> tiles(layer.GetTiles().begin(), layer.GetTiles().end());
        std::sort(tiles.begin(), tiles.end(), [](const auto& a, const auto& b)
            { return a.first.y == b.first.y ? a.first.x < b.first.x : a.first.y < b.first.y; });
        for (const auto& entry : tiles)
            file << "TILE " << entry.first.x << ' ' << entry.first.y << ' ' << entry.second.id << ' '
                 << (entry.second.collidable ? 1 : 0) << '\n';
    }
    return file.good();
}

bool Tilemap::Load(const std::string& path)
{
    std::ifstream file(path);
    if (!file) return false;
    std::string magic, tilesetPath;
    int version = 0, loadedTileSize = 0;
    std::size_t layerCount = 0;
    if (!(file >> magic >> version >> std::quoted(tilesetPath) >> loadedTileSize >> layerCount) ||
        magic != "RAYLIB_TILEMAP" || version != 1 || loadedTileSize < 1 || loadedTileSize > 4096 || layerCount > 64) return false;

    std::vector<TilemapLayer> loadedLayers;
    for (std::size_t index = 0; index < layerCount; ++index)
    {
        std::string marker, name;
        int order = 0;
        std::size_t tileCount = 0;
        if (!(file >> marker >> order >> std::quoted(name) >> tileCount) || marker != "LAYER" || name.empty() || tileCount > 2000000) return false;
        loadedLayers.emplace_back(name, order);
        TilemapLayer& layer = loadedLayers.back();
        for (std::size_t tileIndex = 0; tileIndex < tileCount; ++tileIndex)
        {
            int x = 0, y = 0, id = -1, collision = 0;
            if (!(file >> marker >> x >> y >> id >> collision) || marker != "TILE" || id < 0 ||
                (collision != 0 && collision != 1) || std::abs(x) > 1000000 || std::abs(y) > 1000000) return false;
            layer.SetTile({x, y}, {id, collision != 0});
        }
    }
    std::string extra;
    if (file >> extra) return false;
    if (loadedLayers.empty())
    {
        loadedLayers.emplace_back("Background", 0);
        loadedLayers.emplace_back("Ground", 1);
        loadedLayers.emplace_back("Objects", 2);
        loadedLayers.emplace_back("Foreground", 3);
    }

    Tileset loadedTileset;
    if (!tilesetPath.empty() && !loadedTileset.Load(tilesetPath, loadedTileSize)) return false;
    if (loadedTileset.IsLoaded())
    {
        for (const TilemapLayer& layer : loadedLayers)
            for (const auto& entry : layer.GetTiles())
                if (entry.second.id >= loadedTileset.GetTileCount()) return false;
    }
    tileset.Swap(loadedTileset);
    this->tileSize = loadedTileSize;
    layers = std::move(loadedLayers);
    return true;
}
