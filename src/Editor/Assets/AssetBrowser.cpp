#include "AssetBrowser.h"

#include "../../Engine/Rendering/TextureManager.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace
{
    constexpr float PANEL_WIDTH = 360.0f;
    constexpr float PANEL_HEIGHT = 250.0f;
    constexpr float TILE_WIDTH = 108.0f;
    constexpr float TILE_HEIGHT = 84.0f;
    constexpr float TILE_GAP = 8.0f;
    constexpr int COLUMNS = 3;
}

AssetBrowser::AssetBrowser(Scene& sceneReference)
    : scene(sceneReference),
      panel{260.0f, 0.0f, PANEL_WIDTH, PANEL_HEIGHT},
      scrollRow(0),
      initialized(false)
{
}

AssetBrowser::~AssetBrowser()
{
    Shutdown();
}

void AssetBrowser::Initialize()
{
    panel.y = static_cast<float>(GetScreenHeight()) - PANEL_HEIGHT - 12.0f;
    ScanAssets();
    initialized = true;
}

void AssetBrowser::ScanAssets()
{
    assets.clear();

    const std::filesystem::path directory("resources/sprites");
    std::error_code error;
    if (!std::filesystem::is_directory(directory, error))
    {
        TraceLog(LOG_WARNING, "AssetBrowser: Sprite directory not found: %s", directory.string().c_str());
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory, error))
    {
        if (error)
        {
            break;
        }
        if (!entry.is_regular_file(error))
        {
            continue;
        }

        std::string extension = entry.path().extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
            [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
        if (extension != ".png" && extension != ".jpg" && extension != ".jpeg" && extension != ".bmp")
        {
            continue;
        }

        const std::string path = entry.path().generic_string();
        Texture2D* texture = TextureManager::GetInstance().LoadTextureAsset(path);
        if (texture != nullptr)
        {
            assets.push_back({path, texture});
        }
    }

    std::sort(assets.begin(), assets.end(), [](const AssetEntry& left, const AssetEntry& right)
    {
        return left.path < right.path;
    });
}

void AssetBrowser::Update()
{
    panel.y = static_cast<float>(GetScreenHeight()) - PANEL_HEIGHT - 12.0f;
    if (!IsMouseOver())
    {
        return;
    }

    const float wheel = GetMouseWheelMove();
    const int rowCount = static_cast<int>((assets.size() + COLUMNS - 1) / COLUMNS);
    const int visibleRows = 2;
    const int maxScroll = std::max(0, rowCount - visibleRows);
    if (wheel != 0.0f)
    {
        scrollRow = std::clamp(scrollRow - static_cast<int>(wheel), 0, maxScroll);
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    const Vector2 mouse = GetMousePosition();
    for (std::size_t i = 0; i < assets.size(); ++i)
    {
        const int row = static_cast<int>(i) / COLUMNS - scrollRow;
        const int column = static_cast<int>(i) % COLUMNS;
        if (row < 0 || row >= visibleRows)
        {
            continue;
        }

        const Rectangle tile = {
            panel.x + 10.0f + column * (TILE_WIDTH + TILE_GAP),
            panel.y + 38.0f + row * (TILE_HEIGHT + TILE_GAP),
            TILE_WIDTH,
            TILE_HEIGHT
        };
        if (CheckCollisionPointRec(mouse, tile))
        {
            ApplyAsset(assets[i]);
            return;
        }
    }
}

void AssetBrowser::ApplyAsset(const AssetEntry& asset)
{
    GameObject* object = scene.GetSelectedObject();
    if (object == nullptr)
    {
        return;
    }

    if (object->SetSprite(asset.path))
    {
        // A selected static asset replaces the object's current animation.
        object->GetAnimation().Unload();
    }
}

void AssetBrowser::Draw() const
{
    DrawRectangleRec(panel, Color{35, 35, 35, 245});
    DrawRectangleLinesEx(panel, 1.0f, WHITE);
    DrawText("ASSET BROWSER", static_cast<int>(panel.x + 10.0f), static_cast<int>(panel.y + 10.0f), 16, WHITE);

    if (scene.GetSelectedObject() == nullptr)
    {
        DrawText("Select an object, then click a sprite", static_cast<int>(panel.x + 12.0f),
            static_cast<int>(panel.y + 120.0f), 14, LIGHTGRAY);
    }
    else if (assets.empty())
    {
        DrawText("No image assets found in resources/sprites", static_cast<int>(panel.x + 12.0f),
            static_cast<int>(panel.y + 120.0f), 13, LIGHTGRAY);
    }

    const Vector2 mouse = GetMousePosition();
    for (std::size_t i = 0; i < assets.size(); ++i)
    {
        const int row = static_cast<int>(i) / COLUMNS - scrollRow;
        const int column = static_cast<int>(i) % COLUMNS;
        if (row < 0 || row >= 2)
        {
            continue;
        }

        const Rectangle tile = {
            panel.x + 10.0f + column * (TILE_WIDTH + TILE_GAP),
            panel.y + 38.0f + row * (TILE_HEIGHT + TILE_GAP),
            TILE_WIDTH,
            TILE_HEIGHT
        };
        const bool hovered = CheckCollisionPointRec(mouse, tile);
        DrawRectangleRec(tile, hovered ? Color{75, 95, 125, 255} : Color{55, 55, 55, 255});
        DrawRectangleLinesEx(tile, 1.0f, hovered ? SKYBLUE : DARKGRAY);

        if (assets[i].texture != nullptr && assets[i].texture->width > 0 && assets[i].texture->height > 0)
        {
            const float scale = std::min(56.0f / assets[i].texture->width, 48.0f / assets[i].texture->height);
            const float width = assets[i].texture->width * scale;
            const float height = assets[i].texture->height * scale;
            DrawTexture(*assets[i].texture,
                static_cast<int>(tile.x + (tile.width - width) * 0.5f),
                static_cast<int>(tile.y + 5.0f + (48.0f - height) * 0.5f), WHITE);
        }

        std::string name = std::filesystem::path(assets[i].path).filename().string();
        if (name.size() > 17)
        {
            name = name.substr(0, 14) + "...";
        }
        DrawText(name.c_str(), static_cast<int>(tile.x + 4.0f), static_cast<int>(tile.y + 59.0f), 10, WHITE);
    }

    if (assets.size() > 6)
    {
        DrawText("Mouse wheel: scroll", static_cast<int>(panel.x + 10.0f), static_cast<int>(panel.y + panel.height - 15.0f), 10, LIGHTGRAY);
    }
}

bool AssetBrowser::IsMouseOver() const
{
    return CheckCollisionPointRec(GetMousePosition(), panel);
}

void AssetBrowser::Shutdown()
{
    if (!initialized)
    {
        return;
    }

    for (const AssetEntry& asset : assets)
    {
        TextureManager::GetInstance().ReleaseTexture(asset.path);
    }
    assets.clear();
    initialized = false;
}
