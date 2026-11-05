#pragma once

#include "../Core/Component.h"
#include "raylib.h"

#include <string>

class AudioSource final : public Component
{
public:
    ~AudioSource() override;
    bool SetClip(const std::string& path);
    void Unload();
    bool IsLoaded() const;
    void Play();
    void Stop();
    bool IsPlaying() const;
    void SetVolume(float value);
    float GetVolume() const;
    void SetPitch(float value);
    float GetPitch() const;
    const std::string& GetClipPath() const;
    std::unique_ptr<Component> Clone() const override;

protected:
    void OnDestroy() override;

private:
    Sound* sound = nullptr;
    std::string path;
    float volume = 1.0f;
    float pitch = 1.0f;
};
