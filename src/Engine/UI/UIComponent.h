#pragma once
#include "raylib.h"
#include "../Core/Component.h"
#include <string>
#include <functional>

class UIComponent : public Component
{
public:
    Vector2 size{160.0f, 40.0f};
    Vector2 anchor{0.5f, 0.5f};
    Vector2 offset{};
    Rectangle GetScreenRect() const;
    void SetLayoutRect(Rectangle rect) { layoutRect = rect; hasLayoutRect = true; }
    void ClearLayoutRect() { hasLayoutRect = false; }
private:
    Rectangle layoutRect{};
    bool hasLayoutRect = false;
};

class UICanvas : public Component
{
public:
    Vector2 referenceResolution{1280.0f, 720.0f};
    bool scaleWithScreen = true;
};

class UIImageComponent : public UIComponent
{
public:
    ~UIImageComponent() override;
    bool Load(const std::string& path);
    void Unload();
    void Draw() const;
private:
    Texture2D texture{};
};

class UITextComponent : public UIComponent
{
public:
    std::string text = "Text";
    int fontSize = 24;
    void Draw() const;
};

class UIProgressBar : public UIComponent
{
public:
    float value = 100.0f;
    float maximum = 100.0f;
    Color fillColor{75, 190, 105, 255};
    Color backgroundColor{45, 48, 55, 255};
    void Draw() const;
};

class UIButtonComponent : public UIComponent
{
public:
    std::string label = "Button";
    Color normalColor{55, 75, 110, 255};
    Color hoverColor{75, 115, 175, 255};
    Color pressedColor{35, 50, 80, 255};
    bool hovered = false;
    bool pressed = false;
    std::function<void()> onClick;
    bool ConsumeClick();
    void Update(float deltaTime) override;
    void Draw() const;
private:
    bool clicked = false;
    bool armed = false;
};

class UIInputField : public UIComponent
{
public:
    std::string text;
    std::string placeholder = "Enter text...";
    bool focused = false;
    void Update(float deltaTime) override;
    void Draw() const;
};

class UILayoutGroup : public Component
{
public:
    enum class Direction { Horizontal, Vertical };
    Direction direction = Direction::Vertical;
    float spacing = 8.0f;
    float padding = 8.0f;
    Vector2 itemSize{180.0f, 40.0f};
};
