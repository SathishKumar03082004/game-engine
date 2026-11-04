#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"

class AssetBrowser;

class TilemapEditor
{
public:
    TilemapEditor(Scene& scene, AssetBrowser& assetBrowser);
    void Update(Vector2 mouseWorld);
    void Draw() const;
    bool IsActive() const;
    bool IsMouseOver() const;

private:
    enum class Tool { Brush, Eraser, RectFill, Bucket };
    Scene& scene;
    AssetBrowser& assetBrowser;
    Rectangle panel{};
    bool active = false;
    bool collisionEnabled = false;
    bool painting = false;
    Tool tool = Tool::Brush;
    int selectedTile = 0;
    int paletteScrollRow = 0;
    std::size_t activeLayer = 1;
    TileCoordinate rectangleStart{};

    void PaintAt(TileCoordinate coordinate);
    void FillRectangle(TileCoordinate end);
    void BucketFill(TileCoordinate start);
};
