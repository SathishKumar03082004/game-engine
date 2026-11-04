
#pragma once

#include "raylib.h"
#include <string>

class AnimationComponent
{
public:
    AnimationComponent() = default;
    ~AnimationComponent();

    // Prevent accidental copying of an owned GPU texture.
    AnimationComponent(const AnimationComponent&) = delete;
    AnimationComponent& operator=(const AnimationComponent&) = delete;

    bool LoadSpriteSheet(
        const std::string& path,
        int frameWidth,
        int frameHeight,
        int frameCount
    );

    void Unload();

    void Update(float deltaTime);

    void Draw(
        Vector2 position,
        Vector2 scale,
        float rotation
    ) const;

    void Play();
    void Pause();
    void Stop();
    void Reset();

    void SetFPS(float fps);
    float GetFPS() const;

    void SetLooping(bool looping);
    bool IsLooping() const;
    void SetCurrentFrame(int frame);

    bool IsLoaded() const;
    bool IsPlaying() const;

    int GetCurrentFrame() const;
    int GetFrameCount() const;

    int GetFrameWidth() const;
    int GetFrameHeight() const;

    const std::string& GetFilePath() const;

private:
    Texture2D texture{};

    std::string filePath;

    int frameWidth = 0;
    int frameHeight = 0;
    int frameCount = 0;
    int currentFrame = 0;

    float fps = 8.0f;
    float frameDuration = 1.0f / 8.0f;
    float frameTimer = 0.0f;

    bool loaded = false;
    bool playing = false;
    bool looping = true;
};
