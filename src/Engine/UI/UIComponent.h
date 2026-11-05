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

class RectTransformComponent : public Component
{
public:
    Vector2 anchorMin{0.5f, 0.5f};
    Vector2 anchorMax{0.5f, 0.5f};
    Vector2 pivot{0.5f, 0.5f};
    Vector2 sizeDelta{160.0f, 40.0f};
    Vector2 anchoredPosition{};
    Rectangle GetScreenRect() const;
    std::unique_ptr<Component> Clone() const override;
};

class UICanvas : public Component
{
public:
    enum class RenderMode { ScreenSpaceOverlay, ScreenSpaceCamera, WorldSpace };
    enum class ScaleMode { ConstantPixelSize, ScaleWithScreenSize };
    RenderMode renderMode = RenderMode::ScreenSpaceOverlay;
    ScaleMode scaleMode = ScaleMode::ScaleWithScreenSize;
    Vector2 referenceResolution{1280.0f, 720.0f};
    bool scaleWithScreen = true;
    float scaleFactor = 1.0f;
    float matchWidthOrHeight = 0.0f;
    float referencePixelsPerUnit = 100.0f;
    int sortingOrder = 0;
    float GetScaleFactor() const;
    std::unique_ptr<Component> Clone() const override;
};

class UIEventSystem : public Component
{
public:
    bool sendNavigationEvents = true;
    std::unique_ptr<Component> Clone() const override;
};

class UIImageComponent : public UIComponent
{
public:
    ~UIImageComponent() override;
    bool Load(const std::string& path);
    void Unload();
    void Draw() const;
    bool IsLoaded() const { return texture.id != 0; }
    const std::string& GetPath() const { return sourcePath; }
    std::unique_ptr<Component> Clone() const override;
private:
    Texture2D texture{};
    std::string sourcePath;
};

class UITextComponent : public UIComponent
{
public:
    std::string text = "Text";
    int fontSize = 24;
    void Draw() const;
    std::unique_ptr<Component> Clone() const override;
};

class UIProgressBar : public UIComponent
{
public:
    float value = 100.0f;
    float maximum = 100.0f;
    Color fillColor{75, 190, 105, 255};
    Color backgroundColor{45, 48, 55, 255};
    void Draw() const;
    std::unique_ptr<Component> Clone() const override;
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
    bool focused = false;
    std::function<void()> onClick;
    bool ConsumeClick();
    void Update(float deltaTime) override;
    void Draw() const;
    std::unique_ptr<Component> Clone() const override;
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
    std::unique_ptr<Component> Clone() const override;
};

class UILayoutGroup : public Component
{
public:
    enum class Direction { Horizontal, Vertical };
    Direction direction = Direction::Vertical;
    float spacing = 8.0f;
    float padding = 8.0f;
    Vector2 itemSize{180.0f, 40.0f};
    std::unique_ptr<Component> Clone() const override;
};
