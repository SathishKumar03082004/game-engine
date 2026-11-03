#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"

#include <string>
#include <vector>

class AssetBrowser
{
public:
    explicit AssetBrowser(Scene& sceneReference);
    ~AssetBrowser();

    void Initialize();
    void Update();
    void Draw() const;
    void Shutdown();

    bool IsMouseOver() const;

private:
    struct AssetEntry
    {
        std::string path;
        Texture2D* texture = nullptr;
    };

    Scene& scene;
    Rectangle panel;
    std::vector<AssetEntry> assets;
    int scrollRow;
    bool initialized;

    void ScanAssets();
    void ApplyAsset(const AssetEntry& asset);
};
