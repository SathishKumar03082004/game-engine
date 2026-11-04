#pragma once

#include "raylib.h"

#include <filesystem>
#include <string>
#include <vector>

class AssetBrowser
{
public:
    AssetBrowser();
    ~AssetBrowser();

    void Initialize();
    void Update();
    void Draw() const;
    void Shutdown();

    bool IsMouseOver() const;

private:
    struct Entry
    {
        std::filesystem::path path;
        std::string name;
        Texture2D* thumbnail = nullptr;
        bool isDirectory = false;
    };

    std::filesystem::path projectRoot;
    std::filesystem::path currentDirectory;
    Rectangle panel;
    Rectangle parentButton;
    std::vector<Entry> entries;
    std::string selectedPath;
    int scrollRow;
    bool initialized;

    void ScanDirectory();
    void ReleaseThumbnails();
    void NavigateTo(const std::filesystem::path& directory);
    void HandleEntryClick(Vector2 mouse);
    bool IsInsideProject(const std::filesystem::path& path) const;
    static bool IsImageFile(const std::filesystem::path& path);
    static std::string FormatDisplayPath(
        const std::filesystem::path& root,
        const std::filesystem::path& path
    );
};
