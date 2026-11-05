#include "UIComponent.h"
#include "../Core/InputManager.h"
#include "../Scene/GameObject.h"
#include <algorithm>
#include <cmath>

Rectangle UIComponent::GetScreenRect() const
{
    if (hasLayoutRect) return layoutRect;
    if (GetOwner() != nullptr)
    {
        const RectTransformComponent* transform = GetOwner()->GetComponent<RectTransformComponent>();
        if (transform != nullptr) return transform->GetScreenRect();
    }
    const float sw = static_cast<float>(GetScreenWidth()), sh = static_cast<float>(GetScreenHeight());
    float scale = 1.0f;
    GameObject* root = GetOwner();
    while (root != nullptr && root->GetParent() != nullptr) root = root->GetParent();
    const UICanvas* canvas = root == nullptr ? nullptr : root->GetComponent<UICanvas>();
    if (canvas != nullptr && canvas->scaleWithScreen)
        scale = std::min(sw / canvas->referenceResolution.x, sh / canvas->referenceResolution.y);
    return {anchor.x * sw - size.x * scale * 0.5f + offset.x * scale, anchor.y * sh - size.y * scale * 0.5f + offset.y * scale, size.x * scale, size.y * scale};
}
Rectangle RectTransformComponent::GetScreenRect() const
{
    const float sw = static_cast<float>(GetScreenWidth());
    const float sh = static_cast<float>(GetScreenHeight());
    const GameObject* root = GetOwner();
    while (root != nullptr && root->GetParent() != nullptr) root = root->GetParent();
    const UICanvas* canvas = root == nullptr ? nullptr : root->GetComponent<UICanvas>();
    const float scale = canvas == nullptr ? 1.0f : canvas->GetScaleFactor();
    if (GetOwner() == root && canvas != nullptr) return {0.0f, 0.0f, sw, sh};
    Rectangle parentRect{0.0f, 0.0f, sw, sh};
    if (GetOwner() != nullptr && GetOwner()->GetParent() != nullptr)
    {
        const RectTransformComponent* parentTransform = GetOwner()->GetParent()->GetComponent<RectTransformComponent>();
        if (parentTransform != nullptr) parentRect = parentTransform->GetScreenRect();
    }
    const float width = (anchorMax.x - anchorMin.x) * parentRect.width + sizeDelta.x * scale;
    const float height = (anchorMax.y - anchorMin.y) * parentRect.height + sizeDelta.y * scale;
    const float anchorX = parentRect.x + anchorMin.x * parentRect.width + (anchorMax.x - anchorMin.x) * parentRect.width * pivot.x;
    const float anchorY = parentRect.y + anchorMin.y * parentRect.height + (anchorMax.y - anchorMin.y) * parentRect.height * pivot.y;
    return {anchorX + anchoredPosition.x * scale - pivot.x * width,
        anchorY + anchoredPosition.y * scale - pivot.y * height, width, height};
}
float UICanvas::GetScaleFactor() const
{
    if (!scaleWithScreen || scaleMode == ScaleMode::ConstantPixelSize) return std::max(0.01f, scaleFactor);
    const float widthScale = static_cast<float>(GetScreenWidth()) / std::max(1.0f, referenceResolution.x);
    const float heightScale = static_cast<float>(GetScreenHeight()) / std::max(1.0f, referenceResolution.y);
    const float match = std::clamp(matchWidthOrHeight, 0.0f, 1.0f);
    const float logScale = std::log2(widthScale) * (1.0f - match) + std::log2(heightScale) * match;
    return std::exp2(logScale);
}
std::unique_ptr<Component> RectTransformComponent::Clone() const
{
    auto clone = std::make_unique<RectTransformComponent>();
    clone->anchorMin = anchorMin; clone->anchorMax = anchorMax; clone->pivot = pivot;
    clone->sizeDelta = sizeDelta; clone->anchoredPosition = anchoredPosition;
    return clone;
}
std::unique_ptr<Component> UICanvas::Clone() const
{
    auto clone = std::make_unique<UICanvas>();
    clone->renderMode = renderMode; clone->scaleMode = scaleMode; clone->referenceResolution = referenceResolution;
    clone->scaleWithScreen = scaleWithScreen; clone->scaleFactor = scaleFactor; clone->matchWidthOrHeight = matchWidthOrHeight;
    clone->referencePixelsPerUnit = referencePixelsPerUnit; clone->sortingOrder = sortingOrder;
    return clone;
}
std::unique_ptr<Component> UIEventSystem::Clone() const
{
    auto clone = std::make_unique<UIEventSystem>(); clone->sendNavigationEvents = sendNavigationEvents; return clone;
}
UIImageComponent::~UIImageComponent() { Unload(); }
void UIImageComponent::Unload() { if (texture.id != 0) { UnloadTexture(texture); texture = {}; } sourcePath.clear(); }
bool UIImageComponent::Load(const std::string& path)
{
    if (texture.id) UnloadTexture(texture);
    texture = LoadTexture(path.c_str());
    sourcePath = texture.id == 0 ? std::string{} : path;
    return texture.id != 0;
}
std::unique_ptr<Component> UIImageComponent::Clone() const
{
    auto clone = std::make_unique<UIImageComponent>();
    clone->size = size; clone->anchor = anchor; clone->offset = offset;
    if (IsLoaded()) clone->Load(sourcePath);
    return clone;
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
std::unique_ptr<Component> UITextComponent::Clone() const
{
    auto clone = std::make_unique<UITextComponent>(); clone->size=size; clone->anchor=anchor; clone->offset=offset; clone->text=text; clone->fontSize=fontSize; return clone;
}
void UIProgressBar::Draw() const
{
    const Rectangle rect = GetScreenRect();
    DrawRectangleRec(rect, backgroundColor);
    const float ratio = maximum > 0.0f ? std::clamp(value / maximum, 0.0f, 1.0f) : 0.0f;
    DrawRectangleRec({rect.x, rect.y, rect.width * ratio, rect.height}, fillColor);
    DrawRectangleLinesEx(rect, 1.0f, WHITE);
}
std::unique_ptr<Component> UIProgressBar::Clone() const
{
    auto clone = std::make_unique<UIProgressBar>(); clone->size=size; clone->anchor=anchor; clone->offset=offset;
    clone->value=value; clone->maximum=maximum; clone->fillColor=fillColor; clone->backgroundColor=backgroundColor; return clone;
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
    if (focused && (input.IsKeyPressed(KEY_ENTER) || input.IsKeyPressed(KEY_SPACE)))
    {
        clicked = true;
        if (onClick) onClick();
    }
}
bool UIButtonComponent::ConsumeClick() { const bool value = clicked; clicked = false; return value; }
std::unique_ptr<Component> UIButtonComponent::Clone() const
{
    auto clone = std::make_unique<UIButtonComponent>(); clone->size=size; clone->anchor=anchor; clone->offset=offset;
    clone->label=label; clone->normalColor=normalColor; clone->hoverColor=hoverColor; clone->pressedColor=pressedColor; return clone;
}
void UIButtonComponent::Draw() const
{
    const Rectangle rect = GetScreenRect();
    DrawRectangleRounded(rect, 0.12f, 8, pressed ? pressedColor : (hovered ? hoverColor : normalColor));
    DrawRectangleRoundedLinesEx(rect, 0.12f, 8, focused ? 2.5f : 1.0f, focused ? YELLOW : SKYBLUE);
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
    if (input.IsKeyPressed(KEY_ESCAPE) || input.IsKeyPressed(KEY_ENTER)) focused = false;
}
void UIInputField::Draw() const
{
    const Rectangle rect = GetScreenRect();
    DrawRectangleRec(rect, focused ? Color{42,48,60,255} : Color{28,32,40,255});
    DrawRectangleLinesEx(rect, 2, focused ? SKYBLUE : GRAY);
    const std::string& shown = text.empty() ? placeholder : text;
    DrawText(shown.c_str(), static_cast<int>(rect.x + 10), static_cast<int>(rect.y + (rect.height - 20) * 0.5f), 20, text.empty() ? LIGHTGRAY : WHITE);
}
std::unique_ptr<Component> UIInputField::Clone() const
{
    auto clone = std::make_unique<UIInputField>(); clone->size=size; clone->anchor=anchor; clone->offset=offset;
    clone->text=text; clone->placeholder=placeholder; return clone;
}
std::unique_ptr<Component> UILayoutGroup::Clone() const
{
    auto clone = std::make_unique<UILayoutGroup>(); clone->direction=direction; clone->spacing=spacing; clone->padding=padding; clone->itemSize=itemSize; return clone;
}
