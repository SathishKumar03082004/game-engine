#pragma once

#include "raylib.h"
#include "../../Engine/Resources/AssetReference.h"

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
    bool IsDraggingAsset() const;
    std::string ConsumeDroppedAssetPath();

private:
    struct Entry
    {
        std::filesystem::path path;
        AssetReference assetReference;
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
    std::filesystem::path selectedFile;
    AssetReference selectedAsset;
    Texture2D* previewTexture;
    Texture2D* dragTexture;
    std::string dragCandidatePath;
    std::string droppedAssetPath;
    Vector2 dragStartPosition;
    int scrollRow;
    bool initialized;
    bool isDraggingAsset;

    void ScanDirectory();
    void ReleaseThumbnails();
    void ReleasePreview();
    void NavigateTo(const std::filesystem::path& directory);
    void HandleEntryClick(Vector2 mouse);
    void SelectFile(const Entry& entry);
    bool IsInsideProject(const std::filesystem::path& path) const;
    static bool IsImageFile(const std::filesystem::path& path);
    static std::string FormatDisplayPath(
        const std::filesystem::path& root,
        const std::filesystem::path& path
    );
};
