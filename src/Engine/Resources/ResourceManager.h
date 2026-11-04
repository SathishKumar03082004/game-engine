#pragma once

#include "AssetReference.h"
#include "raylib.h"

#include <string>
#include <unordered_map>

class ResourceManager
{
public:
    static ResourceManager& GetInstance();

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    bool Initialize();
    bool RegisterAsset(const AssetReference& reference);
    const AssetReference* FindAsset(const std::string& id) const;

    Texture2D* LoadTextureById(const std::string& id);
    Texture2D* LoadTextureAsset(const AssetReference& reference);
    Texture2D* LoadTextureAsset(const std::string& path);
    void ReleaseTexture(const AssetReference& reference);
    void ReleaseTextureById(const std::string& id);
    void ReleaseTexture(const std::string& path);

    void Shutdown();
    bool IsLoaded(const std::string& path) const;
    int GetLoadedTextureCount() const;

private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    struct TextureAsset
    {
        Texture2D texture{};
        unsigned int referenceCount = 0;
    };

    static std::string NormalizePath(const std::string& path);

    std::unordered_map<std::string, TextureAsset> textures;
    std::unordered_map<std::string, AssetReference> assetsById;
    bool initialized = false;
};
