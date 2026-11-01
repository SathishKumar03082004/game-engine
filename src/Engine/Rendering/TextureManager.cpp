#include "TextureManager.h"

#include <iostream>

TextureManager& TextureManager::GetInstance()
{
    static TextureManager instance;
    return instance;
}

bool TextureManager::Initialize()
{
    if (initialized)
    {
        return true;
    }

    if (!IsWindowReady())
    {
        TraceLog(
            LOG_ERROR,
            "TextureManager: Window must be initialized first."
        );

        return false;
    }

    initialized = true;

    TraceLog(
        LOG_INFO,
        "TextureManager initialized."
    );

    return true;
}

Texture2D* TextureManager::LoadTextureAsset(
    const std::string& path
)
{
    if (!initialized)
    {
        TraceLog(
            LOG_ERROR,
            "TextureManager: Initialize before loading textures."
        );

        return nullptr;
    }

    if (path.empty())
    {
        return nullptr;
    }

    // Reuse an existing texture.
    auto iterator = textures.find(path);

    if (iterator != textures.end())
    {
        iterator->second.referenceCount++;

        TraceLog(
            LOG_INFO,
            "TextureManager: Reusing texture: %s",
            path.c_str()
        );

        return &iterator->second.texture;
    }

    // Check whether the file exists.
    if (!FileExists(path.c_str()))
    {
        TraceLog(
            LOG_WARNING,
            "TextureManager: File not found: %s",
            path.c_str()
        );

        return nullptr;
    }

    Texture2D texture = LoadTexture(path.c_str());

    if (texture.id == 0)
    {
        TraceLog(
            LOG_ERROR,
            "TextureManager: Failed to load: %s",
            path.c_str()
        );

        return nullptr;
    }

    SetTextureFilter(
        texture,
        TEXTURE_FILTER_BILINEAR
    );

    TextureAsset asset;

    asset.texture = texture;
    asset.referenceCount = 1;

    auto result = textures.emplace(path, asset);

    TraceLog(
        LOG_INFO,
        "TextureManager: Loaded: %s",
        path.c_str()
    );

    return &result.first->second.texture;
}

void TextureManager::ReleaseTexture(
    const std::string& path
)
{
    auto iterator = textures.find(path);

    if (iterator == textures.end())
    {
        return;
    }

    if (iterator->second.referenceCount > 0)
    {
        iterator->second.referenceCount--;
    }

    // Only unload when nobody references this asset.
    if (iterator->second.referenceCount == 0)
    {
        UnloadTexture(iterator->second.texture);

        textures.erase(iterator);

        TraceLog(
            LOG_INFO,
            "TextureManager: Unloaded: %s",
            path.c_str()
        );
    }
}

bool TextureManager::IsLoaded(
    const std::string& path
) const
{
    return textures.find(path) != textures.end();
}

int TextureManager::GetLoadedTextureCount() const
{
    return static_cast<int>(textures.size());
}

void TextureManager::Shutdown()
{
    for (auto& pair : textures)
    {
        UnloadTexture(pair.second.texture);
    }

    textures.clear();

    initialized = false;

    TraceLog(
        LOG_INFO,
        "TextureManager shutdown."
    );
}