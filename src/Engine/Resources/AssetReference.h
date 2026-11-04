#pragma once

#include <string>
#include <utility>

enum class AssetType
{
    Unknown,
    TextureAsset,
    Audio,
    FontAsset,
    Data
};

struct AssetReference
{
    std::string id;
    std::string path;
    AssetType type = AssetType::Unknown;

    AssetReference() = default;
    AssetReference(std::string assetId, std::string assetPath, AssetType assetType)
        : id(std::move(assetId)), path(std::move(assetPath)), type(assetType) {}

    bool IsValid() const { return !path.empty(); }
    explicit operator bool() const { return IsValid(); }

    static AssetReference FromPath(std::string path, AssetType type = AssetType::Unknown)
    {
        return AssetReference({}, std::move(path), type);
    }
};
