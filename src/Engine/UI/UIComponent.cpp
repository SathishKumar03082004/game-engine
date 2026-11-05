#include "UIComponent.h"
#include "../Core/InputManager.h"
#include "../Scene/GameObject.h"
#include <algorithm>

Rectangle UIComponent::GetScreenRect() const
{
    if (hasLayoutRect) return layoutRect;
    const float sw = static_cast<float>(GetScreenWidth()), sh = static_cast<float>(GetScreenHeight());
    float scale = 1.0f;
    GameObject* root = GetOwner();
    while (root != nullptr && root->GetParent() != nullptr) root = root->GetParent();
    const UICanvas* canvas = root == nullptr ? nullptr : root->GetComponent<UICanvas>();
    if (canvas != nullptr && canvas->scaleWithScreen)
        scale = std::min(sw / canvas->referenceResolution.x, sh / canvas->referenceResolution.y);
    return {anchor.x * sw - size.x * scale * 0.5f + offset.x * scale, anchor.y * sh - size.y * scale * 0.5f + offset.y * scale, size.x * scale, size.y * scale};
}
UIImageComponent::~UIImageComponent() { Unload(); }
void UIImageComponent::Unload() { if (texture.id != 0) { UnloadTexture(texture); texture = {}; } }
bool UIImageComponent::Load(const std::string& path)
{
    if (texture.id) UnloadTexture(texture);
    texture = LoadTexture(path.c_str());
    return texture.id != 0;
}
void UIImageComponent::Draw() const
{
    const Rectangle rect = GetScreenRect();
    if (texture.id) DrawTexturePro(texture, {0,0,(float)texture.width,(float)texture.height}, rect, {0,0}, 0, WHITE);
    else { DrawRectangleRec(rect, DARKGRAY); DrawRectangleLinesEx(rect, 1, LIGHTGRAY); }
}
void UITextComponent::Draw() const
{
    const Rectangle rect = GetScreenRect();
    DrawText(text.c_str(), static_cast<int>(rect.x), static_cast<int>(rect.y), fontSize, WHITE);
}
void UIProgressBar::Draw() const
{
    const Rectangle rect = GetScreenRect();
    DrawRectangleRec(rect, backgroundColor);
    const float ratio = maximum > 0.0f ? std::clamp(value / maximum, 0.0f, 1.0f) : 0.0f;
    DrawRectangleRec({rect.x, rect.y, rect.width * ratio, rect.height}, fillColor);
    DrawRectangleLinesEx(rect, 1.0f, WHITE);
}
void UIButtonComponent::Update(float)
{
    const auto& input = InputManager::GetInstance();
    hovered = CheckCollisionPointRec(input.GetMousePosition(), GetScreenRect());
    pressed = hovered && input.IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    if (hovered && input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) armed = true;
    if (input.IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        if (armed && hovered) { clicked = true; if (onClick) onClick(); }
        armed = false;
    }
}
bool UIButtonComponent::ConsumeClick() { const bool value = clicked; clicked = false; return value; }
void UIButtonComponent::Draw() const
{
    const Rectangle rect = GetScreenRect();
    DrawRectangleRounded(rect, 0.12f, 8, pressed ? pressedColor : (hovered ? hoverColor : normalColor));
    DrawRectangleRoundedLinesEx(rect, 0.12f, 8, 1.0f, SKYBLUE);
    const int font = 18, width = MeasureText(label.c_str(), font);
    DrawText(label.c_str(), static_cast<int>(rect.x + (rect.width - width) * 0.5f), static_cast<int>(rect.y + (rect.height - font) * 0.5f), font, WHITE);
}
void UIInputField::Update(float)
{
    const auto& input = InputManager::GetInstance();
    if (input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) focused = CheckCollisionPointRec(input.GetMousePosition(), GetScreenRect());
    if (!focused) return;
    int code = input.GetCharPressed();
    while (code > 0) { if (code >= 32 && code <= 126 && text.size() < 128) text.push_back(static_cast<char>(code)); code = input.GetCharPressed(); }
    if (input.IsKeyPressed(KEY_BACKSPACE) && !text.empty()) text.pop_back();
    if (input.IsKeyPressed(KEY_ESCAPE)) focused = false;
}
void UIInputField::Draw() const
{
    const Rectangle rect = GetScreenRect();
    DrawRectangleRec(rect, focused ? Color{42,48,60,255} : Color{28,32,40,255});
    DrawRectangleLinesEx(rect, 2, focused ? SKYBLUE : GRAY);
    const std::string& shown = text.empty() ? placeholder : text;
    DrawText(shown.c_str(), static_cast<int>(rect.x + 10), static_cast<int>(rect.y + (rect.height - 20) * 0.5f), 20, text.empty() ? LIGHTGRAY : WHITE);
}
