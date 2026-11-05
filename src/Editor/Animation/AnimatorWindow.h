#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"
#include <string>

class AnimatorWindow
{
public:
    explicit AnimatorWindow(Scene& scene);
    void SetVisible(bool value) { visible = value; }
    bool IsVisible() const { return visible; }
    bool IsMouseOver() const;
    void Update();
    void Draw() const;

private:
    enum class LinkMode { Select, Completion, Bool, Trigger };
    Scene& scene;
    Rectangle bounds{};
    bool visible = false;
    bool consumeMouseThisFrame = false;
    float graphScroll = 0.0f;
    bool dragging = false;
    Vector2 dragOffset{};
    LinkMode linkMode = LinkMode::Select;
    std::string sourceState;
    std::string parameterName;
    std::string parameterBuffer;
    bool parameterFieldFocused = false;
    bool addAsTrigger = false;
    std::string status;

    Rectangle CloseButton() const;
    Rectangle StateRect(std::size_t index) const;
    const char* ModeLabel() const;
    void AddParameter();
};
