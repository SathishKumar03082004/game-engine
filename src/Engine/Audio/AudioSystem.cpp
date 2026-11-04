#include "AudioSystem.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

AudioSystem& AudioSystem::GetInstance()
{
    static AudioSystem system;
    return system;
}

bool AudioSystem::Initialize()
{
    if (!IsAudioDeviceReady()) InitAudioDevice();
    initialized = IsAudioDeviceReady();
    return initialized;
}

Sound* AudioSystem::LoadSoundAsset(const std::string& path)
{
    if (!initialized || path.empty() || !FileExists(path.c_str())) return nullptr;
    std::error_code error;
    std::filesystem::path absolute = std::filesystem::absolute(path, error);
    const std::string key = (error ? std::filesystem::path(path) : absolute).lexically_normal().generic_string();
    auto found = sounds.find(key);
    if (found != sounds.end())
    {
        ++found->second.references;
        return &found->second.sound;
    }
    Sound sound = LoadSound(path.c_str());
    if (sound.frameCount == 0) return nullptr;
    auto inserted = sounds.emplace(key, SoundAsset{sound, 1});
    return &inserted.first->second.sound;
}

void AudioSystem::ReleaseSound(const std::string& path)
{
    if (path.empty()) return;
    std::error_code error;
    const std::filesystem::path absolute = std::filesystem::absolute(path, error);
    const std::string key = (error ? std::filesystem::path(path) : absolute).lexically_normal().generic_string();
    auto found = sounds.find(key);
    if (found == sounds.end()) return;
    if (found->second.references > 0) --found->second.references;
    if (found->second.references == 0)
    {
        UnloadSound(found->second.sound);
        sounds.erase(found);
    }
}

void AudioSystem::SetListenerPosition(Vector2 position)
{
    if (std::isfinite(position.x) && std::isfinite(position.y)) listenerPosition = position;
}
Vector2 AudioSystem::GetListenerPosition() const { return listenerPosition; }

void AudioSystem::PlayAt(Sound& sound, Vector2 sourcePosition, float volume, float pitch) const
{
    if (!initialized || !std::isfinite(volume) || !std::isfinite(pitch) || pitch <= 0.0f) return;
    const float dx = sourcePosition.x - listenerPosition.x;
    const float dy = sourcePosition.y - listenerPosition.y;
    const float distance = std::sqrt(dx * dx + dy * dy);
    const float attenuation = std::clamp(1.0f - distance / 1200.0f, 0.0f, 1.0f);
    const float pan = std::clamp(dx / 1200.0f, -1.0f, 1.0f);
    SetSoundVolume(sound, std::clamp(volume, 0.0f, 1.0f) * attenuation);
    SetSoundPitch(sound, pitch);
    SetSoundPan(sound, pan);
    PlaySound(sound);
}

void AudioSystem::Shutdown()
{
    for (auto& entry : sounds) UnloadSound(entry.second.sound);
    sounds.clear();
    if (IsAudioDeviceReady()) CloseAudioDevice();
    initialized = false;
}
