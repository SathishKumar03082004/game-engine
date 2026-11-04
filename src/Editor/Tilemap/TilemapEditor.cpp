#include "TilemapEditor.h"

#include "../Assets/AssetBrowser.h"
#include "../../Engine/Core/InputManager.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

namespace
{
    constexpr float PANEL_WIDTH = 660.0f;
    constexpr float PANEL_HEIGHT = 330.0f;
    constexpr float PALETTE_CELL = 30.0f;
    constexpr int PALETTE_COLUMNS = 21;

    Rectangle Button(float x, float y, float width, float height)
    {
        return {x, y, width, height};
    }
    bool SameCoordinate(TileCoordinate a, TileCoordinate b) { return a.x == b.x && a.y == b.y; }
}

TilemapEditor::TilemapEditor(Scene& sceneRef, AssetBrowser& browser)
    : scene(sceneRef), assetBrowser(browser), panel{260.0f, 0.0f, PANEL_WIDTH, PANEL_HEIGHT}
{
}

bool TilemapEditor::IsActive() const { return active; }
bool TilemapEditor::IsMouseOver() const
{
    return active && CheckCollisionPointRec(InputManager::GetInstance().GetMousePosition(), panel);
}

void TilemapEditor::Update(Vector2 mouseWorld)
{
    InputManager& input = InputManager::GetInstance();
    if (input.IsKeyPressed(KEY_F6)) active = !active;
    if (!active) return;

    panel.y = static_cast<float>(GetScreenHeight()) - PANEL_HEIGHT - 12.0f;
    Tilemap& tilemap = scene.GetTilemap();
    const Vector2 mouseScreen = input.GetMousePosition();
    const Rectangle paletteArea{panel.x + 8.0f, panel.y + 128.0f, panel.width - 16.0f, panel.height - 138.0f};
    if (IsMouseOver() && CheckCollisionPointRec(mouseScreen, paletteArea) && tilemap.GetTileset().IsLoaded())
    {
        const int visibleRows = std::max(1, static_cast<int>(paletteArea.height / PALETTE_CELL));
        const int totalRows = (tilemap.GetTileset().GetTileCount() + PALETTE_COLUMNS - 1) / PALETTE_COLUMNS;
        const int maxRow = std::max(0, totalRows - visibleRows);
        const int wheel = static_cast<int>(input.GetMouseWheelMove());
        if (wheel != 0) paletteScrollRow = std::clamp(paletteScrollRow - wheel, 0, maxRow);
    }
    if (IsMouseOver() && input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (CheckCollisionPointRec(mouseScreen, Button(panel.x + 10.0f, panel.y + 30.0f, 145.0f, 28.0f)))
        {
            const std::string path = assetBrowser.GetSelectedFilePath();
            if (!path.empty()) tilemap.SetTileset(path, tilemap.GetTileSize());
        }
        else if (CheckCollisionPointRec(mouseScreen, Button(panel.x + 160.0f, panel.y + 30.0f, 32.0f, 28.0f)))
        {
            const int size = std::max(8, tilemap.GetTileSize() - 8);
            if (tilemap.GetTileset().IsLoaded()) tilemap.SetTileset(tilemap.GetTileset().GetPath(), size);
            else tilemap.SetTileSize(size);
        }
        else if (CheckCollisionPointRec(mouseScreen, Button(panel.x + 225.0f, panel.y + 30.0f, 32.0f, 28.0f)))
        {
            const int size = std::min(256, tilemap.GetTileSize() + 8);
            if (tilemap.GetTileset().IsLoaded()) tilemap.SetTileset(tilemap.GetTileset().GetPath(), size);
            else tilemap.SetTileSize(size);
        }
        else if (CheckCollisionPointRec(mouseScreen, Button(panel.x + 265.0f, panel.y + 30.0f, 110.0f, 28.0f)))
            collisionEnabled = !collisionEnabled;
        else
        {
            const char* layers[] = {"Background", "Ground", "Objects", "Foreground"};
            bool layerClicked = false;
            for (std::size_t i = 0; i < 4; ++i)
            {
                if (CheckCollisionPointRec(mouseScreen, Button(panel.x + 10.0f + i * 112.0f, panel.y + 64.0f, 108.0f, 26.0f)))
                {
                    activeLayer = i;
                    layerClicked = true;
                    break;
                }
            }
            if (!layerClicked)
            {
                const char* tools[] = {"Brush", "Eraser", "Rect", "Bucket"};
                for (int i = 0; i < 4; ++i)
                    if (CheckCollisionPointRec(mouseScreen, Button(panel.x + 10.0f + i * 78.0f, panel.y + 96.0f, 72.0f, 26.0f)))
                    {
                        tool = static_cast<Tool>(i);
                        break;
                    }

                const Rectangle palette{panel.x + 8.0f, panel.y + 128.0f, panel.width - 16.0f, panel.height - 138.0f};
                if (CheckCollisionPointRec(mouseScreen, palette) && tilemap.GetTileset().IsLoaded())
                {
                    const int column = static_cast<int>((mouseScreen.x - palette.x) / PALETTE_CELL);
                    const int row = static_cast<int>((mouseScreen.y - palette.y) / PALETTE_CELL) + paletteScrollRow;
                    const int tile = row * PALETTE_COLUMNS + column;
                    if (tile >= 0 && tile < tilemap.GetTileset().GetTileCount()) selectedTile = tile;
                }
            }
        }
    }

    if (!input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !input.IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        !input.IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return;
    if (IsMouseOver()) return;

    const TileCoordinate coordinate = tilemap.WorldToTile(mouseWorld);
    if (input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (tool == Tool::RectFill)
        {
            rectangleStart = coordinate;
            painting = true;
        }
        else if (tool == Tool::Bucket) BucketFill(coordinate);
        else
        {
            PaintAt(coordinate);
            painting = true;
        }
    }
    else if (input.IsMouseButtonDown(MOUSE_BUTTON_LEFT) && painting &&
             (tool == Tool::Brush || tool == Tool::Eraser)) PaintAt(coordinate);
    else if (input.IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && painting && tool == Tool::RectFill)
        FillRectangle(coordinate);
    if (input.IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) painting = false;
}

void TilemapEditor::PaintAt(TileCoordinate coordinate)
{
    TilemapLayer* layer = scene.GetTilemap().GetLayer(activeLayer);
    if (layer == nullptr) return;
    if (tool == Tool::Eraser) layer->RemoveTile(coordinate);
    else if (scene.GetTilemap().GetTileset().IsLoaded()) layer->SetTile(coordinate, {selectedTile, collisionEnabled});
}

void TilemapEditor::FillRectangle(TileCoordinate end)
{
    TilemapLayer* layer = scene.GetTilemap().GetLayer(activeLayer);
    if (layer == nullptr || !scene.GetTilemap().GetTileset().IsLoaded()) return;
    const int minX = std::min(rectangleStart.x, end.x), maxX = std::max(rectangleStart.x, end.x);
    const int minY = std::min(rectangleStart.y, end.y), maxY = std::max(rectangleStart.y, end.y);
    if (static_cast<long long>(maxX - minX + 1) * (maxY - minY + 1) > 1000000) return;
    for (int y = minY; y <= maxY; ++y)
        for (int x = minX; x <= maxX; ++x) layer->SetTile({x, y}, {selectedTile, collisionEnabled});
}

void TilemapEditor::BucketFill(TileCoordinate start)
{
    TilemapLayer* layer = scene.GetTilemap().GetLayer(activeLayer);
    if (layer == nullptr || !scene.GetTilemap().GetTileset().IsLoaded() ||
        std::abs(start.x) > 128 || std::abs(start.y) > 128) return;
    const Tile original = layer->GetTile(start);
    if (original.id == selectedTile && original.collidable == collisionEnabled) return;
    std::queue<TileCoordinate> pending;
    std::unordered_set<TileCoordinate, TileCoordinateHash> visited;
    pending.push(start);
    while (!pending.empty() && visited.size() < 200000)
    {
        const TileCoordinate coordinate = pending.front();
        pending.pop();
        if (std::abs(coordinate.x) > 128 || std::abs(coordinate.y) > 128 || !visited.insert(coordinate).second) continue;
        const Tile current = layer->GetTile(coordinate);
        if (current.id != original.id || current.collidable != original.collidable) continue;
        layer->SetTile(coordinate, {selectedTile, collisionEnabled});
        pending.push({coordinate.x + 1, coordinate.y});
        pending.push({coordinate.x - 1, coordinate.y});
        pending.push({coordinate.x, coordinate.y + 1});
        pending.push({coordinate.x, coordinate.y - 1});
    }
}

void TilemapEditor::Draw() const
{
    if (!active) return;
    const Tilemap& tilemap = scene.GetTilemap();
    const Vector2 mouse = InputManager::GetInstance().GetMousePosition();
    DrawRectangleRec(panel, Color{28, 30, 34, 250});
    DrawRectangleLinesEx(panel, 1.0f, SKYBLUE);
    DrawText("TILEMAP EDITOR (F6)", static_cast<int>(panel.x + 10.0f), static_cast<int>(panel.y + 8.0f), 14, WHITE);

    const char* tilesetButtonLabel = tilemap.GetTileset().IsLoaded() ? "Reload Selected Texture" : "Use Selected Texture";
    const Rectangle useButton{panel.x + 10.0f, panel.y + 30.0f, 145.0f, 28.0f};
    DrawRectangleRec(useButton, DARKGRAY);
    DrawText(tilesetButtonLabel, static_cast<int>(useButton.x + 6.0f), static_cast<int>(useButton.y + 8.0f), 10, WHITE);
    DrawRectangleRec({panel.x + 160.0f, panel.y + 30.0f, 32.0f, 28.0f}, DARKGRAY);
    DrawText("-", static_cast<int>(panel.x + 172.0f), static_cast<int>(panel.y + 35.0f), 16, WHITE);
    DrawText(TextFormat("%dpx", tilemap.GetTileSize()), static_cast<int>(panel.x + 196.0f), static_cast<int>(panel.y + 38.0f), 12, WHITE);
    DrawRectangleRec({panel.x + 225.0f, panel.y + 30.0f, 32.0f, 28.0f}, DARKGRAY);
    DrawText("+", static_cast<int>(panel.x + 236.0f), static_cast<int>(panel.y + 35.0f), 16, WHITE);
    DrawRectangleRec({panel.x + 265.0f, panel.y + 30.0f, 110.0f, 28.0f}, collisionEnabled ? DARKGREEN : DARKGRAY);
    DrawText(collisionEnabled ? "Collision: ON" : "Collision: OFF", static_cast<int>(panel.x + 272.0f), static_cast<int>(panel.y + 39.0f), 11, WHITE);
    if (tilemap.GetTileset().IsLoaded())
        DrawText(TextFormat("%d x %d tiles", tilemap.GetTileset().GetColumns(), tilemap.GetTileset().GetRows()),
            static_cast<int>(panel.x + 390.0f), static_cast<int>(panel.y + 39.0f), 11, LIGHTGRAY);
    else
        DrawText("Select an image in Asset Browser first", static_cast<int>(panel.x + 390.0f), static_cast<int>(panel.y + 39.0f), 10, LIGHTGRAY);

    const char* layers[] = {"Background", "Ground", "Objects", "Foreground"};
    for (std::size_t i = 0; i < 4; ++i)
    {
        const Rectangle button{panel.x + 10.0f + i * 112.0f, panel.y + 64.0f, 108.0f, 26.0f};
        DrawRectangleRec(button, activeLayer == i ? SKYBLUE : DARKGRAY);
        DrawText(layers[i], static_cast<int>(button.x + 9.0f), static_cast<int>(button.y + 7.0f), 11, WHITE);
    }
    const char* tools[] = {"Brush", "Eraser", "Rect Fill", "Bucket"};
    for (int i = 0; i < 4; ++i)
    {
        const Rectangle button{panel.x + 10.0f + i * 78.0f, panel.y + 96.0f, 72.0f, 26.0f};
        DrawRectangleRec(button, static_cast<int>(tool) == i ? DARKBLUE : DARKGRAY);
        DrawText(tools[i], static_cast<int>(button.x + 8.0f), static_cast<int>(button.y + 7.0f), 11, WHITE);
    }
    DrawText(TextFormat("Selected tile: %d", selectedTile), static_cast<int>(panel.x + 335.0f), static_cast<int>(panel.y + 102.0f), 11, YELLOW);

    const Rectangle palette{panel.x + 8.0f, panel.y + 128.0f, panel.width - 16.0f, panel.height - 138.0f};
    DrawRectangleRec(palette, Color{18, 18, 18, 255});
    if (!tilemap.GetTileset().IsLoaded())
    {
        DrawText("Choose a texture in Asset Browser, then click Use Selected Texture.",
            static_cast<int>(palette.x + 12.0f), static_cast<int>(palette.y + 12.0f), 12, LIGHTGRAY);
        return;
    }
    BeginScissorMode(static_cast<int>(palette.x), static_cast<int>(palette.y),
        static_cast<int>(palette.width), static_cast<int>(palette.height));
    for (int id = 0; id < tilemap.GetTileset().GetTileCount(); ++id)
    {
        const int column = id % PALETTE_COLUMNS;
        const int row = id / PALETTE_COLUMNS - paletteScrollRow;
        if (row < 0) continue;
        const Rectangle destination{palette.x + 2.0f + column * PALETTE_CELL,
                                    palette.y + 2.0f + row * PALETTE_CELL, 26.0f, 26.0f};
        DrawRectangleRec(destination, id == selectedTile ? SKYBLUE : Color{45, 45, 45, 255});
        tilemap.GetTileset().DrawTile(id, {destination.x + 2.0f, destination.y + 2.0f, 22.0f, 22.0f});
    }
    EndScissorMode();
    static_cast<void>(mouse);
}
