
#include "AnimationComponent.h"

#include <algorithm>

AnimationComponent::~AnimationComponent()
{
    Unload();
}

bool AnimationComponent::LoadSpriteSheet(
    const std::string& path,
    int newFrameWidth,
    int newFrameHeight,
    int newFrameCount
)
{
    if (path.empty() ||
        newFrameWidth <= 0 ||
        newFrameHeight <= 0 ||
        newFrameCount <= 0)
    {
        return false;
    }

    // Load the new texture before releasing the old one.
    Texture2D newTexture = LoadTexture(path.c_str());

    if (newTexture.id == 0)
    {
        return false;
    }

    if (newTexture.width < newFrameWidth * newFrameCount ||
        newTexture.height < newFrameHeight)
    {
        UnloadTexture(newTexture);
        return false;
    }

    Unload();

    texture = newTexture;
    filePath = path;

    frameWidth = newFrameWidth;
    frameHeight = newFrameHeight;
    frameCount = newFrameCount;

    currentFrame = 0;
    frameTimer = 0.0f;

    loaded = true;
    playing = false;

    return true;
}

void AnimationComponent::Unload()
{
    if (loaded)
    {
        UnloadTexture(texture);
    }

    texture = {};
    filePath.clear();

    frameWidth = 0;
    frameHeight = 0;
    frameCount = 0;
    currentFrame = 0;

    frameTimer = 0.0f;

    loaded = false;
    playing = false;
}

void AnimationComponent::Update(float deltaTime)
{
    if (!loaded || !playing || frameCount <= 1)
    {
        return;
    }

    if (deltaTime < 0.0f)
    {
        return;
    }

    frameTimer += deltaTime;

    while (frameTimer >= frameDuration)
    {
        frameTimer -= frameDuration;

        currentFrame++;

        if (currentFrame >= frameCount)
        {
            if (looping)
            {
                currentFrame = 0;
            }
            else
            {
                currentFrame = frameCount - 1;
                playing = false;
                frameTimer = 0.0f;
                break;
            }
        }
    }
}

void AnimationComponent::Draw(
    Vector2 position,
    Vector2 scale,
    float rotation
) const
{
    if (!loaded)
    {
        return;
    }

    Rectangle source =
    {
        static_cast<float>(currentFrame * frameWidth),
        0.0f,
        static_cast<float>(frameWidth),
        static_cast<float>(frameHeight)
    };

    // Negative scale flips the sprite.
    if (scale.x < 0.0f)
    {
        source.width *= -1.0f;
    }

    if (scale.y < 0.0f)
    {
        source.height *= -1.0f;
    }

    float destinationWidth =
        static_cast<float>(frameWidth) * scale.x;

    float destinationHeight =
        static_cast<float>(frameHeight) * scale.y;

    Rectangle destination =
    {
        position.x,
        position.y,
        destinationWidth,
        destinationHeight
    };

    Vector2 origin =
    {
        destinationWidth * 0.5f,
        destinationHeight * 0.5f
    };

    DrawTexturePro(
        texture,
        source,
        destination,
        origin,
        rotation,
        WHITE
    );
}

void AnimationComponent::Play()
{
    if (loaded)
    {
        if (!playing && currentFrame == frameCount - 1 && !looping)
        {
            currentFrame = 0;
            frameTimer = 0.0f;
        }

        playing = true;
    }
}

void AnimationComponent::Pause()
{
    playing = false;
}

void AnimationComponent::Stop()
{
    playing = false;
    currentFrame = 0;
    frameTimer = 0.0f;
}

void AnimationComponent::Reset()
{
    currentFrame = 0;
    frameTimer = 0.0f;
}

void AnimationComponent::SetFPS(float newFPS)
{
    fps = std::max(1.0f, newFPS);
    frameDuration = 1.0f / fps;
}

float AnimationComponent::GetFPS() const
{
    return fps;
}

void AnimationComponent::SetLooping(bool value)
{
    looping = value;
}

bool AnimationComponent::IsLooping() const
{
    return looping;
}

bool AnimationComponent::IsLoaded() const
{
    return loaded;
}

bool AnimationComponent::IsPlaying() const
{
    return playing;
}

int AnimationComponent::GetCurrentFrame() const
{
    return currentFrame;
}

int AnimationComponent::GetFrameCount() const
{
    return frameCount;
}

int AnimationComponent::GetFrameWidth() const
{
    return frameWidth;
}

int AnimationComponent::GetFrameHeight() const
{
    return frameHeight;
}

const std::string& AnimationComponent::GetFilePath() const
{
    return filePath;
}
