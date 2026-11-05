#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"
#include <string>

class AnimationWindow
{
public:
    explicit AnimationWindow(Scene& scene);
    void SetVisible(bool value) { visible = value; }
    bool IsVisible() const { return visible; }
    void SetPreviewUpdates(bool value) { previewUpdates = value; }
    bool IsMouseOver() const;
    void Update();
    void Draw() const;

private:
    enum class TextField { None, ClipName, SpritePath };
    Scene& scene;
    Rectangle bounds{};
    bool visible = false;
    bool previewUpdates = true;
    bool consumeMouseThisFrame = false;
    float clipScroll = 0.0f;
    int timelineStart = 0;
    bool dragging = false;
    Vector2 dragOffset{};
    bool addClipOpen = false;
    bool loopNewClip = true;
    TextField activeField = TextField::None;
    std::string textBuffer;
    std::string newClipName = "New Clip";
    std::string newSpritePath;
    int newFrameWidth = 44;
    int newFrameHeight = 39;
    int newFrameCount = 6;
    float newFPS = 8.0f;
    std::string status;

    Rectangle CloseButton() const;
    void DrawButton(Rectangle rect, const char* label, bool selected = false) const;
    void CommitTextField();
    bool AddClip();
};
