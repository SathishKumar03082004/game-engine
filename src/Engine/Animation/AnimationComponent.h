#pragma once

#include "../Core/Component.h"
#include "raylib.h"

#include <map>
#include <string>
#include <vector>

struct AnimationClipInfo
{
    std::string name;
    std::string filePath;
    int frameWidth = 0;
    int frameHeight = 0;
    int frameCount = 0;
    int currentFrame = 0;
    float fps = 8.0f;
    bool looping = true;
    bool playing = false;
};

class AnimationComponent final : public Component
{
public:
    AnimationComponent() = default;
    ~AnimationComponent() override;

    AnimationComponent(const AnimationComponent&) = delete;
    AnimationComponent& operator=(const AnimationComponent&) = delete;

    bool AddAnimation(
        const std::string& name,
        const std::string& path,
        int frameWidth,
        int frameHeight,
        int frameCount,
        float fps = 8.0f,
        bool looping = true
    );
    bool RemoveAnimation(const std::string& name);
    void ClearAnimations();
    bool HasAnimation(const std::string& name) const;
    std::vector<AnimationClipInfo> GetAnimations() const;
    bool Play(const std::string& name, bool restart = true);
    const std::string& GetCurrentAnimationName() const;

    // Compatibility helpers for the original single-sheet API.
    bool LoadSpriteSheet(const std::string& path, int frameWidth, int frameHeight, int frameCount);
    void Unload();
    void Update(float deltaTime) override;
    void Draw(Vector2 position, Vector2 scale, float rotation) const;
    void Play();
    void Pause();
    void Stop();
    void Reset();
    void SetFPS(float fps);
    float GetFPS() const;
    void SetLooping(bool looping);
    bool IsLooping() const;
    void SetFrameCount(int frameCount);
    void SetCurrentFrame(int frame);
    bool IsLoaded() const;
    bool IsPlaying() const;
    bool IsFinished() const;
    int GetCurrentFrame() const;
    int GetFrameCount() const;
    int GetFrameWidth() const;
    int GetFrameHeight() const;
    const std::string& GetFilePath() const;

protected:
    void OnDestroy() override;

private:
    struct Clip
    {
        Texture2D texture{};
        std::string filePath;
        int frameWidth = 0;
        int frameHeight = 0;
        int frameCount = 0;
        int currentFrame = 0;
        float fps = 8.0f;
        float frameDuration = 1.0f / 8.0f;
        float frameTimer = 0.0f;
        bool playing = false;
        bool looping = true;
        bool finished = false;
    };

    std::map<std::string, Clip> clips;
    std::string activeName;

    Clip* GetActiveClip();
    const Clip* GetActiveClip() const;
    static AnimationClipInfo ToInfo(const std::string& name, const Clip& clip);
};
