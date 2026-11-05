#include "AudioListener.h"
#include "AudioSystem.h"
#include "../Scene/GameObject.h"

void AudioListener::Update(float deltaTime)
{
    static_cast<void>(deltaTime);
    if (GetOwner() != nullptr) AudioSystem::GetInstance().SetListenerPosition(GetOwner()->GetWorldPosition());
}
void AudioListener::OnEnable()
{
    if (GetOwner() != nullptr) AudioSystem::GetInstance().SetListenerPosition(GetOwner()->GetWorldPosition());
}
void AudioListener::OnDisable() { AudioSystem::GetInstance().SetListenerPosition({0.0f, 0.0f}); }
void AudioListener::OnDestroy() { AudioSystem::GetInstance().SetListenerPosition({0.0f, 0.0f}); }
std::unique_ptr<Component> AudioListener::Clone() const { return std::make_unique<AudioListener>(); }
