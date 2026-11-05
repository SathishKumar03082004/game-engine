#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"
#include <string>
#include <unordered_map>

class AnimationController;
class GameObject;

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
    float parameterScroll = 0.0f;
    int selectedTransition = -1;
    int transitionScroll = 0;
    bool dragging = false;
    bool stateWasDragged = false;
    Vector2 dragOffset{};
    Vector2 stateDragStart{};
    LinkMode linkMode = LinkMode::Select;
    bool expectedBoolValue = true;
    std::string draggingState;
    std::unordered_map<std::string, Vector2> statePositions;
    GameObject* positionsOwner = nullptr;
    std::string sourceState;
    std::string parameterName;
    std::string parameterBuffer;
    bool parameterFieldFocused = false;
    bool addAsTrigger = false;
    std::string status;

    Rectangle CloseButton() const;
    Rectangle StateRect(std::size_t index, const std::string& state) const;
    const char* ModeLabel() const;
    void AddParameter();
    void ActivateState(AnimationController& controller, const std::string& state);
};
