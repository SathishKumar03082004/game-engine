#include "AnimationComponent.h"
#include "../Resources/ResourceManager.h"

#include <algorithm>
#include <cmath>

AnimationComponent::~AnimationComponent()
{
    Unload();
}

bool AnimationComponent::AddAnimation(
    const std::string& name,
    const std::string& path,
    int newFrameWidth,
    int newFrameHeight,
    int newFrameCount,
    float newFPS,
    bool newLooping
)
{
    return AddAnimation(name, AssetReference::FromPath(path, AssetType::TextureAsset),
        newFrameWidth, newFrameHeight, newFrameCount, newFPS, newLooping);
}

bool AnimationComponent::AddAnimation(
    const std::string& name,
    const AssetReference& reference,
    int newFrameWidth,
    int newFrameHeight,
    int newFrameCount,
    float newFPS,
    bool newLooping
)
{
    if (name.empty() || !reference.IsValid() || newFrameWidth <= 0 ||
        newFrameHeight <= 0 || newFrameCount <= 0 ||
        !std::isfinite(newFPS) || newFPS <= 0.0f)
    {
        return false;
    }

    Texture2D* texture = ResourceManager::GetInstance().LoadTextureAsset(reference);
    if (texture == nullptr) return false;
    if (static_cast<long long>(texture->width) < static_cast<long long>(newFrameWidth) * newFrameCount ||
        texture->height < newFrameHeight)
    {
        ResourceManager::GetInstance().ReleaseTexture(reference);
        return false;
    }

    Clip replacement;
    replacement.texture = texture;
    replacement.assetReference = reference;
    replacement.frameWidth = newFrameWidth;
    replacement.frameHeight = newFrameHeight;
    replacement.frameCount = newFrameCount;
    replacement.fps = std::max(1.0f, newFPS);
    replacement.frameDuration = 1.0f / replacement.fps;
    replacement.looping = newLooping;

    auto found = clips.find(name);
    if (found != clips.end())
    {
        ResourceManager::GetInstance().ReleaseTexture(found->second.assetReference);
        found->second = std::move(replacement);
    }
    else
    {
        clips.emplace(name, std::move(replacement));
    }

    if (activeName.empty()) activeName = name;
    return true;
}

bool AnimationComponent::RemoveAnimation(const std::string& name)
{
    const auto found = clips.find(name);
    if (found == clips.end()) return false;
    ResourceManager::GetInstance().ReleaseTexture(found->second.assetReference);
    clips.erase(found);
    if (activeName == name)
    {
        activeName = clips.empty() ? std::string{} : clips.begin()->first;
    }
    return true;
}

void AnimationComponent::ClearAnimations()
{
    for (auto& entry : clips)
    {
        if (entry.second.texture != nullptr)
            ResourceManager::GetInstance().ReleaseTexture(entry.second.assetReference);
    }
    clips.clear();
    activeName.clear();
}

bool AnimationComponent::HasAnimation(const std::string& name) const
{
    return clips.find(name) != clips.end();
}

std::vector<AnimationClipInfo> AnimationComponent::GetAnimations() const
{
    std::vector<AnimationClipInfo> result;
    result.reserve(clips.size());
    for (const auto& entry : clips) result.push_back(ToInfo(entry.first, entry.second));
    return result;
}

bool AnimationComponent::Play(const std::string& name, bool restart)
{
    auto found = clips.find(name);
    if (found == clips.end()) return false;
    if (activeName != name || restart)
    {
        found->second.currentFrame = 0;
        found->second.frameTimer = 0.0f;
        found->second.finished = false;
    }
    activeName = name;
    found->second.playing = true;
    found->second.finished = false;
    return true;
}

const std::string& AnimationComponent::GetCurrentAnimationName() const
{
    return activeName;
}

bool AnimationComponent::LoadSpriteSheet(
    const std::string& path,
    int frameWidth,
    int frameHeight,
    int frameCount
)
{
    if (!AddAnimation("Default", path, frameWidth, frameHeight, frameCount)) return false;
    activeName = "Default";
    return true;
}

void AnimationComponent::Unload()
{
    ClearAnimations();
}

void AnimationComponent::OnDestroy()
{
    Unload();
}

AnimationComponent::Clip* AnimationComponent::GetActiveClip()
{
    auto found = clips.find(activeName);
    return found == clips.end() ? nullptr : &found->second;
}

const AnimationComponent::Clip* AnimationComponent::GetActiveClip() const
{
    auto found = clips.find(activeName);
    return found == clips.end() ? nullptr : &found->second;
}

AnimationClipInfo AnimationComponent::ToInfo(const std::string& name, const Clip& clip)
{
    return {name, clip.assetReference.path, clip.frameWidth, clip.frameHeight, clip.frameCount,
        clip.currentFrame, clip.fps, clip.looping, clip.playing};
}

void AnimationComponent::Update(float deltaTime)
{
    Clip* clip = GetActiveClip();
    if (clip == nullptr || !clip->playing ||
        !std::isfinite(deltaTime) || deltaTime <= 0.0f)
    {
        return;
    }

    if (clip->frameCount <= 1)
    {
        if (!clip->looping)
        {
            clip->frameTimer += deltaTime;
            if (clip->frameTimer >= clip->frameDuration)
            {
                clip->playing = false;
                clip->finished = true;
            }
        }
        return;
    }

    clip->frameTimer += deltaTime;
    while (clip->frameTimer >= clip->frameDuration)
    {
        clip->frameTimer -= clip->frameDuration;
        ++clip->currentFrame;
        if (clip->currentFrame >= clip->frameCount)
        {
            if (clip->looping) clip->currentFrame = 0;
            else
            {
                clip->currentFrame = clip->frameCount - 1;
                clip->playing = false;
                clip->finished = true;
                clip->frameTimer = 0.0f;
                break;
            }
        }
    }
}

void AnimationComponent::Draw(Vector2 position, Vector2 scale, float rotation) const
{
    const Clip* clip = GetActiveClip();
    if (clip == nullptr || clip->texture == nullptr) return;

    Rectangle source = {
        static_cast<float>(clip->currentFrame * clip->frameWidth), 0.0f,
        static_cast<float>(clip->frameWidth), static_cast<float>(clip->frameHeight)
    };
    if (scale.x < 0.0f) source.width *= -1.0f;
    if (scale.y < 0.0f) source.height *= -1.0f;

    const float destinationWidth = clip->frameWidth * scale.x;
    const float destinationHeight = clip->frameHeight * scale.y;
    const Rectangle destination = {position.x, position.y, destinationWidth, destinationHeight};
    const Vector2 origin = {destinationWidth * 0.5f, destinationHeight * 0.5f};
    DrawTexturePro(*clip->texture, source, destination, origin, rotation, WHITE);
}

void AnimationComponent::Play()
{
    Clip* clip = GetActiveClip();
    if (clip == nullptr) return;
    if (!clip->playing && clip->currentFrame == clip->frameCount - 1 && !clip->looping)
    {
        clip->currentFrame = 0;
        clip->frameTimer = 0.0f;
        clip->finished = false;
    }
    clip->playing = true;
    clip->finished = false;
}

void AnimationComponent::Pause()
{
    if (Clip* clip = GetActiveClip()) clip->playing = false;
}

void AnimationComponent::Stop()
{
    if (Clip* clip = GetActiveClip())
    {
        clip->playing = false;
        clip->currentFrame = 0;
        clip->frameTimer = 0.0f;
        clip->finished = false;
    }
}

void AnimationComponent::Reset()
{
    if (Clip* clip = GetActiveClip())
    {
        clip->currentFrame = 0;
        clip->frameTimer = 0.0f;
        clip->finished = false;
    }
}

void AnimationComponent::SetFPS(float fps)
{
    Clip* clip = GetActiveClip();
    if (clip == nullptr || !std::isfinite(fps) || fps <= 0.0f) return;
    clip->fps = std::max(1.0f, fps);
    clip->frameDuration = 1.0f / clip->fps;
}

float AnimationComponent::GetFPS() const
{
    const Clip* clip = GetActiveClip();
    return clip == nullptr ? 0.0f : clip->fps;
}

void AnimationComponent::SetLooping(bool looping)
{
    if (Clip* clip = GetActiveClip()) clip->looping = looping;
}

bool AnimationComponent::IsLooping() const
{
    const Clip* clip = GetActiveClip();
    return clip != nullptr && clip->looping;
}

void AnimationComponent::SetFrameCount(int frameCount)
{
    Clip* clip = GetActiveClip();
    if (clip == nullptr || frameCount <= 0 ||
        clip->texture == nullptr ||
        static_cast<long long>(frameCount) * clip->frameWidth > clip->texture->width) return;
    clip->frameCount = frameCount;
    clip->currentFrame = std::min(clip->currentFrame, frameCount - 1);
    clip->finished = false;
}

void AnimationComponent::SetCurrentFrame(int frame)
{
    if (Clip* clip = GetActiveClip())
    {
        clip->currentFrame = std::clamp(frame, 0, clip->frameCount - 1);
        clip->frameTimer = 0.0f;
        clip->finished = false;
    }
}

bool AnimationComponent::IsLoaded() const { return GetActiveClip() != nullptr; }
bool AnimationComponent::IsPlaying() const { const Clip* c = GetActiveClip(); return c != nullptr && c->playing; }
bool AnimationComponent::IsFinished() const { const Clip* c = GetActiveClip(); return c != nullptr && c->finished; }
int AnimationComponent::GetCurrentFrame() const { const Clip* c = GetActiveClip(); return c == nullptr ? 0 : c->currentFrame; }
int AnimationComponent::GetFrameCount() const { const Clip* c = GetActiveClip(); return c == nullptr ? 0 : c->frameCount; }
int AnimationComponent::GetFrameWidth() const { const Clip* c = GetActiveClip(); return c == nullptr ? 0 : c->frameWidth; }
int AnimationComponent::GetFrameHeight() const { const Clip* c = GetActiveClip(); return c == nullptr ? 0 : c->frameHeight; }
const std::string& AnimationComponent::GetFilePath() const { const Clip* c = GetActiveClip(); static const std::string empty; return c == nullptr ? empty : c->assetReference.path; }
Texture2D* AnimationComponent::GetActiveTexture() const
{
    const Clip* clip = GetActiveClip();
    return clip == nullptr ? nullptr : clip->texture;
}
Rectangle AnimationComponent::GetActiveSourceRect(Vector2 scale) const
{
    const Clip* clip = GetActiveClip();
    if (clip == nullptr) return {};
    Rectangle source{static_cast<float>(clip->currentFrame * clip->frameWidth), 0.0f,
                     static_cast<float>(clip->frameWidth), static_cast<float>(clip->frameHeight)};
    if (scale.x < 0.0f) source.width *= -1.0f;
    if (scale.y < 0.0f) source.height *= -1.0f;
    return source;
}

std::unique_ptr<Component> AnimationComponent::Clone() const
{
    auto clone = std::make_unique<AnimationComponent>();
    const std::string active = GetCurrentAnimationName();
    for (const AnimationClipInfo& info : GetAnimations())
    {
        if (!clone->AddAnimation(info.name, info.filePath, info.frameWidth, info.frameHeight,
                info.frameCount, info.fps, info.looping)) continue;
        if (info.name == active)
        {
            clone->Play(info.name);
            clone->SetCurrentFrame(info.currentFrame);
            if (!info.playing) clone->Pause();
        }
    }
    return clone;
}
