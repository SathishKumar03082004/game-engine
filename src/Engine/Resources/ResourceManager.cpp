#include "ResourceManager.h"

#include <filesystem>

ResourceManager& ResourceManager::GetInstance()
{
    static ResourceManager instance;
    return instance;
}

std::string ResourceManager::NormalizePath(const std::string& path)
{
    if (path.empty()) return {};
    std::error_code error;
    const std::filesystem::path absolute = std::filesystem::absolute(path, error);
    return (error ? std::filesystem::path(path) : absolute).lexically_normal().generic_string();
}

bool ResourceManager::Initialize()
{
    if (initialized) return true;
    if (!IsWindowReady())
    {
        TraceLog(LOG_ERROR, "ResourceManager: Window must be initialized first.");
        return false;
    }
    initialized = true;
    TraceLog(LOG_INFO, "ResourceManager initialized.");
    return true;
}

bool ResourceManager::RegisterAsset(const AssetReference& reference)
{
    if (reference.id.empty() || reference.path.empty()) return false;
    const auto found = assetsById.find(reference.id);
    if (found != assetsById.end())
    {
        return found->second.path == reference.path && found->second.type == reference.type;
    }
    assetsById.emplace(reference.id, reference);
    return true;
}

const AssetReference* ResourceManager::FindAsset(const std::string& id) const
{
    const auto found = assetsById.find(id);
    return found == assetsById.end() ? nullptr : &found->second;
}

Texture2D* ResourceManager::LoadTextureById(const std::string& id)
{
    const AssetReference* reference = FindAsset(id);
    return reference == nullptr ? nullptr : LoadTextureAsset(*reference);
}

Texture2D* ResourceManager::LoadTextureAsset(const AssetReference& reference)
{
    if (!reference.IsValid() ||
        (reference.type != AssetType::Unknown && reference.type != AssetType::TextureAsset))
    {
        return nullptr;
    }
    return LoadTextureAsset(reference.path);
}

Texture2D* ResourceManager::LoadTextureAsset(const std::string& path)
{
    if (!initialized || path.empty())
    {
        TraceLog(LOG_ERROR, "ResourceManager: Initialize before loading textures.");
        return nullptr;
    }

    const std::string key = NormalizePath(path);
    const auto found = textures.find(key);
    if (found != textures.end())
    {
        ++found->second.referenceCount;
        return &found->second.texture;
    }

    if (!FileExists(path.c_str()))
    {
        TraceLog(LOG_WARNING, "ResourceManager: File not found: %s", path.c_str());
        return nullptr;
    }

    Texture2D texture = LoadTexture(path.c_str());
    if (texture.id == 0)
    {
        TraceLog(LOG_ERROR, "ResourceManager: Failed to load texture: %s", path.c_str());
        return nullptr;
    }

    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    TextureAsset asset{texture, 1};
    const auto inserted = textures.emplace(key, asset);
    return &inserted.first->second.texture;
}

void ResourceManager::ReleaseTexture(const AssetReference& reference)
{
    ReleaseTexture(reference.path);
}

void ResourceManager::ReleaseTextureById(const std::string& id)
{
    const AssetReference* reference = FindAsset(id);
    if (reference != nullptr) ReleaseTexture(*reference);
}

void ResourceManager::ReleaseTexture(const std::string& path)
{
    const auto found = textures.find(NormalizePath(path));
    if (found == textures.end()) return;

    if (found->second.referenceCount > 0) --found->second.referenceCount;
    if (found->second.referenceCount == 0)
    {
        UnloadTexture(found->second.texture);
        textures.erase(found);
    }
}

void ResourceManager::Shutdown()
{
    for (auto& entry : textures) UnloadTexture(entry.second.texture);
    textures.clear();
    assetsById.clear();
    initialized = false;
    TraceLog(LOG_INFO, "ResourceManager shutdown.");
}

bool ResourceManager::IsLoaded(const std::string& path) const
{
    return textures.find(NormalizePath(path)) != textures.end();
}

int ResourceManager::GetLoadedTextureCount() const
{
    return static_cast<int>(textures.size());
}
