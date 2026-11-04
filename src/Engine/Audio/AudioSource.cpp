#include "AudioSource.h"
#include "AudioSystem.h"
#include "AudioListener.h"
#include "../Scene/GameObject.h"

#include <algorithm>
#include <cmath>

AudioSource::~AudioSource() { Unload(); }
bool AudioSource::SetClip(const std::string& newPath)
{
    if (newPath.empty()) return false;
    if (newPath == path && sound != nullptr) return true;
    Sound* loaded = AudioSystem::GetInstance().LoadSoundAsset(newPath);
    if (loaded == nullptr) return false;
    Unload();
    sound = loaded;
    path = newPath;
    return true;
}
void AudioSource::Unload()
{
    if (sound != nullptr && !path.empty()) AudioSystem::GetInstance().ReleaseSound(path);
    sound = nullptr;
    path.clear();
}
bool AudioSource::IsLoaded() const { return sound != nullptr; }
void AudioSource::Play()
{
    if (sound == nullptr) return;
    const Vector2 position = GetOwner() == nullptr ? Vector2{0.0f, 0.0f} : GetOwner()->GetWorldPosition();
    AudioSystem::GetInstance().PlayAt(*sound, position, volume, pitch);
}
void AudioSource::Stop() { if (sound != nullptr) StopSound(*sound); }
bool AudioSource::IsPlaying() const { return sound != nullptr && IsSoundPlaying(*sound); }
void AudioSource::SetVolume(float value) { if (std::isfinite(value)) volume = std::clamp(value, 0.0f, 1.0f); }
float AudioSource::GetVolume() const { return volume; }
void AudioSource::SetPitch(float value) { if (std::isfinite(value) && value > 0.0f) pitch = value; }
float AudioSource::GetPitch() const { return pitch; }
const std::string& AudioSource::GetClipPath() const { return path; }
void AudioSource::OnDestroy() { Unload(); }
