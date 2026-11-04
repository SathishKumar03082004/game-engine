#include "AssetBrowser.h"

#include "../../Engine/Rendering/TextureManager.h"

#include <algorithm>
#include <cctype>
#include <system_error>

namespace
{
    constexpr float PANEL_WIDTH = 420.0f;
    constexpr float PANEL_HEIGHT = 300.0f;
    constexpr float TILE_WIDTH = 126.0f;
    constexpr float TILE_HEIGHT = 68.0f;
    constexpr float TILE_GAP = 7.0f;
    constexpr float CONTENT_TOP = 58.0f;
    constexpr int COLUMNS = 3;
    constexpr int VISIBLE_ROWS = 3;

    std::string Lowercase(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
        return value;
    }

    std::string Shorten(const std::string& value, std::size_t maxLength)
    {
        if (value.size() <= maxLength) return value;
        if (maxLength <= 3) return value.substr(0, maxLength);
        return value.substr(0, maxLength - 3) + "...";
    }
}

AssetBrowser::AssetBrowser()
    : panel{260.0f, 0.0f, PANEL_WIDTH, PANEL_HEIGHT},
      parentButton{},
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
    std::error_code error;
    projectRoot = std::filesystem::current_path(error);
    if (error)
    {
        TraceLog(LOG_ERROR, "AssetBrowser: Could not determine project directory.");
        return;
    }

    projectRoot = projectRoot.lexically_normal();
    currentDirectory = projectRoot;
    panel.y = static_cast<float>(GetScreenHeight()) - PANEL_HEIGHT - 12.0f;
    parentButton = {panel.x + PANEL_WIDTH - 52.0f, panel.y + 28.0f, 42.0f, 22.0f};
    initialized = true;
    ScanDirectory();
}

bool AssetBrowser::IsImageFile(const std::filesystem::path& path)
{
    const std::string extension = Lowercase(path.extension().string());
    return extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
           extension == ".bmp" || extension == ".tga";
}

bool AssetBrowser::IsInsideProject(const std::filesystem::path& path) const
{
    const std::filesystem::path relative = path.lexically_normal().lexically_relative(projectRoot);
    if (relative.empty()) return path.lexically_normal() == projectRoot;
    const auto first = relative.begin();
    return first == relative.end() || *first != "..";
}

std::string AssetBrowser::FormatDisplayPath(
    const std::filesystem::path& root,
    const std::filesystem::path& path
)
{
    const std::filesystem::path relative = path.lexically_relative(root);
    return relative.empty() ? "Project" : std::string("Project/") + relative.generic_string();
}

void AssetBrowser::ReleaseThumbnails()
{
    for (const Entry& entry : entries)
    {
        if (entry.thumbnail != nullptr)
        {
            TextureManager::GetInstance().ReleaseTexture(entry.path.string());
        }
    }
    entries.clear();
}

void AssetBrowser::ScanDirectory()
{
    ReleaseThumbnails();
    if (!initialized || !IsInsideProject(currentDirectory)) return;

    std::error_code error;
    std::filesystem::directory_iterator iterator(currentDirectory, error);
    if (error)
    {
        TraceLog(LOG_WARNING, "AssetBrowser: Could not read folder: %s", currentDirectory.string().c_str());
        return;
    }

    for (const auto& item : iterator)
    {
        error.clear();
        if (item.is_symlink(error) || error) continue;

        const std::string name = item.path().filename().string();
        if (name.empty() || name[0] == '.') continue;

        error.clear();
        const bool isDirectory = item.is_directory(error);
        if (error)
        {
            error.clear();
            continue;
        }
        if (isDirectory && (name == "bin" || name == "obj" || name == "build")) continue;

        error.clear();
        if (!isDirectory && !item.is_regular_file(error)) continue;

        Entry entry;
        entry.path = item.path().lexically_normal();
        entry.name = name;
        entry.isDirectory = isDirectory;
        if (!isDirectory && IsImageFile(entry.path))
        {
            entry.thumbnail = TextureManager::GetInstance().LoadTextureAsset(entry.path.string());
        }
        entries.push_back(std::move(entry));
    }

    std::sort(entries.begin(), entries.end(), [](const Entry& left, const Entry& right)
    {
        if (left.isDirectory != right.isDirectory) return left.isDirectory > right.isDirectory;
        return Lowercase(left.name) < Lowercase(right.name);
    });

    scrollRow = std::clamp(scrollRow, 0,
        std::max(0, static_cast<int>((entries.size() + COLUMNS - 1) / COLUMNS) - VISIBLE_ROWS));
}

void AssetBrowser::NavigateTo(const std::filesystem::path& directory)
{
    const std::filesystem::path normalized = directory.lexically_normal();
    if (!IsInsideProject(normalized)) return;

    std::error_code error;
    if (!std::filesystem::is_directory(normalized, error) || error) return;

    currentDirectory = normalized;
    scrollRow = 0;
    ScanDirectory();
}

void AssetBrowser::HandleEntryClick(Vector2 mouse)
{
    for (std::size_t index = 0; index < entries.size(); ++index)
    {
        const int row = static_cast<int>(index) / COLUMNS - scrollRow;
        const int column = static_cast<int>(index) % COLUMNS;
        if (row < 0 || row >= VISIBLE_ROWS) continue;

        const Rectangle tile = {
            panel.x + 10.0f + column * (TILE_WIDTH + TILE_GAP),
            panel.y + CONTENT_TOP + row * (TILE_HEIGHT + TILE_GAP),
            TILE_WIDTH,
            TILE_HEIGHT
        };
        if (!CheckCollisionPointRec(mouse, tile)) continue;

        if (entries[index].isDirectory)
        {
            NavigateTo(entries[index].path);
        }
        else
        {
            selectedPath = FormatDisplayPath(projectRoot, entries[index].path);
        }
        return;
    }
}

void AssetBrowser::Update()
{
    if (!initialized) return;
    panel.y = static_cast<float>(GetScreenHeight()) - PANEL_HEIGHT - 12.0f;
    parentButton = {panel.x + PANEL_WIDTH - 52.0f, panel.y + 28.0f, 42.0f, 22.0f};

    if (!IsMouseOver()) return;

    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f)
    {
        const int rowCount = static_cast<int>((entries.size() + COLUMNS - 1) / COLUMNS);
        const int maxScroll = std::max(0, rowCount - VISIBLE_ROWS);
        scrollRow = std::clamp(scrollRow - static_cast<int>(wheel), 0, maxScroll);
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    const Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, parentButton))
    {
        if (currentDirectory != projectRoot) NavigateTo(currentDirectory.parent_path());
        return;
    }
    HandleEntryClick(mouse);
}

void AssetBrowser::Draw() const
{
    if (!initialized) return;

    DrawRectangleRec(panel, Color{35, 35, 35, 245});
    DrawRectangleLinesEx(panel, 1.0f, WHITE);
    DrawText("PROJECT ASSETS", static_cast<int>(panel.x + 10.0f), static_cast<int>(panel.y + 8.0f), 16, WHITE);

    const std::string location = Shorten(FormatDisplayPath(projectRoot, currentDirectory), 48);
    DrawText(location.c_str(), static_cast<int>(panel.x + 10.0f), static_cast<int>(panel.y + 32.0f), 11, LIGHTGRAY);

    const bool canGoUp = currentDirectory != projectRoot;
    DrawRectangleRec(parentButton, canGoUp ? DARKGRAY : Color{55, 55, 55, 255});
    DrawRectangleLinesEx(parentButton, 1.0f, canGoUp ? LIGHTGRAY : DARKGRAY);
    DrawText("Up", static_cast<int>(parentButton.x + 12.0f), static_cast<int>(parentButton.y + 5.0f), 11,
        canGoUp ? WHITE : GRAY);

    const Vector2 mouse = GetMousePosition();
    for (std::size_t index = 0; index < entries.size(); ++index)
    {
        const int row = static_cast<int>(index) / COLUMNS - scrollRow;
        const int column = static_cast<int>(index) % COLUMNS;
        if (row < 0 || row >= VISIBLE_ROWS) continue;

        const Rectangle tile = {
            panel.x + 10.0f + column * (TILE_WIDTH + TILE_GAP),
            panel.y + CONTENT_TOP + row * (TILE_HEIGHT + TILE_GAP),
            TILE_WIDTH,
            TILE_HEIGHT
        };
        const bool selected = !entries[index].isDirectory &&
            selectedPath == FormatDisplayPath(projectRoot, entries[index].path);
        const bool hovered = CheckCollisionPointRec(mouse, tile);
        DrawRectangleRec(tile, selected ? Color{55, 90, 130, 255} :
            (hovered ? Color{65, 65, 65, 255} : Color{48, 48, 48, 255}));
        DrawRectangleLinesEx(tile, 1.0f, selected ? SKYBLUE : DARKGRAY);

        if (entries[index].isDirectory)
        {
            const float iconX = tile.x + (tile.width - 34.0f) * 0.5f;
            const float iconY = tile.y + 8.0f;
            DrawRectangleRec({iconX, iconY, 15.0f, 7.0f}, GOLD);
            DrawRectangleRec({iconX, iconY + 5.0f, 34.0f, 24.0f}, Color{210, 165, 65, 255});
            DrawRectangleLinesEx({iconX, iconY + 5.0f, 34.0f, 24.0f}, 1.0f, ORANGE);
        }
        else if (entries[index].thumbnail != nullptr &&
                 entries[index].thumbnail->width > 0 && entries[index].thumbnail->height > 0)
        {
            const float scale = std::min(48.0f / entries[index].thumbnail->width,
                                         36.0f / entries[index].thumbnail->height);
            const float imageWidth = entries[index].thumbnail->width * scale;
            const float imageHeight = entries[index].thumbnail->height * scale;
            DrawTexture(*entries[index].thumbnail,
                static_cast<int>(tile.x + (tile.width - imageWidth) * 0.5f),
                static_cast<int>(tile.y + 4.0f + (38.0f - imageHeight) * 0.5f), WHITE);
        }
        else
        {
            DrawRectangleRec({tile.x + 46.0f, tile.y + 7.0f, 34.0f, 34.0f}, Color{90, 100, 115, 255});
            DrawRectangleLinesEx({tile.x + 46.0f, tile.y + 7.0f, 34.0f, 34.0f}, 1.0f, LIGHTGRAY);
            const std::string extension = entries[index].isDirectory ? "DIR" :
                Lowercase(entries[index].path.extension().string());
            const std::string badge = entries[index].isDirectory ? extension : Shorten(extension, 5);
            DrawText(badge.c_str(), static_cast<int>(tile.x + 51.0f), static_cast<int>(tile.y + 20.0f), 9, WHITE);
        }

        DrawText(Shorten(entries[index].name, 19).c_str(), static_cast<int>(tile.x + 4.0f),
            static_cast<int>(tile.y + 48.0f), 10, WHITE);
    }

    if (entries.empty())
    {
        DrawText("This folder is empty.", static_cast<int>(panel.x + 12.0f),
            static_cast<int>(panel.y + CONTENT_TOP + 12.0f), 13, LIGHTGRAY);
    }

    const std::string footer = selectedPath.empty()
        ? "Click folders to browse; click a file to select it."
        : Shorten(selectedPath, 60);
    DrawText(footer.c_str(), static_cast<int>(panel.x + 10.0f),
        static_cast<int>(panel.y + PANEL_HEIGHT - 16.0f), 9, LIGHTGRAY);
}

bool AssetBrowser::IsMouseOver() const
{
    return initialized && CheckCollisionPointRec(GetMousePosition(), panel);
}

void AssetBrowser::Shutdown()
{
    if (!initialized) return;
    ReleaseThumbnails();
    initialized = false;
}
