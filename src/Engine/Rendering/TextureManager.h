#pragma once

#include "raylib.h"

#include <string>
#include <unordered_map>

class TextureManager
{
public:
    static TextureManager& GetInstance();

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    bool Initialize();

    Texture2D* LoadTextureAsset(const std::string& path);

    void ReleaseTexture(const std::string& path);

    void Shutdown();

    bool IsLoaded(const std::string& path) const;

    int GetLoadedTextureCount() const;

private:
    TextureManager() = default;
    ~TextureManager() = default;

    struct TextureAsset
    {
        Texture2D texture{};
        unsigned int referenceCount = 0;
    };

    std::unordered_map<std::string, TextureAsset> textures;

    bool initialized = false;
};