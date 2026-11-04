#pragma once

#include "raylib.h"

#include <string>
#include <unordered_map>

class AudioSystem
{
public:
    static AudioSystem& GetInstance();
    bool Initialize();
    Sound* LoadSoundAsset(const std::string& path);
    void ReleaseSound(const std::string& path);
    void SetListenerPosition(Vector2 position);
    Vector2 GetListenerPosition() const;
    void PlayAt(Sound& sound, Vector2 sourcePosition, float volume, float pitch) const;
    void Shutdown();

private:
    struct SoundAsset { Sound sound{}; unsigned int references = 0; };
    std::unordered_map<std::string, SoundAsset> sounds;
    Vector2 listenerPosition{};
    bool initialized = false;
};
