#pragma once

#include <functional>

struct Tile
{
    int id = -1;
    bool collidable = false;
    bool IsEmpty() const { return id < 0; }
};

struct TileCoordinate
{
    int x = 0;
    int y = 0;
    bool operator==(const TileCoordinate& other) const { return x == other.x && y == other.y; }
};

struct TileCoordinateHash
{
    std::size_t operator()(TileCoordinate value) const noexcept
    {
        const std::size_t x = std::hash<int>{}(value.x);
        const std::size_t y = std::hash<int>{}(value.y);
        return x ^ (y + 0x9e3779b9U + (x << 6U) + (x >> 2U));
    }
};
