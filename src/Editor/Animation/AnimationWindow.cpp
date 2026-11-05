#include "AnimationWindow.h"
#include "../../Engine/Animation/AnimationComponent.h"
#include "../../Engine/Core/InputManager.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr Color WindowBg{42, 43, 46, 255};
    constexpr Color HeaderBg{55, 57, 61, 255};
    constexpr Color FieldBg{30, 31, 34, 255};
    bool Clicked(Rectangle rectangle)
    {
        return InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(InputManager::GetInstance().GetMousePosition(), rectangle);
    }
}

AnimationWindow::AnimationWindow(Scene& sceneReference) : scene(sceneReference)
{
    bounds = {std::max(20.0f, (GetScreenWidth() - 700.0f) * 0.5f), 90.0f, 700.0f, 440.0f};
}

Rectangle AnimationWindow::CloseButton() const { return {bounds.x + bounds.width - 29.0f, bounds.y + 3.0f, 25.0f, 23.0f}; }

bool AnimationWindow::IsMouseOver() const
{
    return consumeMouseThisFrame || (visible && CheckCollisionPointRec(InputManager::GetInstance().GetMousePosition(), bounds));
}

void AnimationWindow::DrawButton(Rectangle rect, const char* label, bool selected) const
{
    DrawRectangleRec(rect, selected ? Color{70, 110, 160, 255} : Color{67, 69, 73, 255});
    DrawRectangleLinesEx(rect, 1.0f, Color{25, 26, 28, 255});
    DrawText(label, static_cast<int>(rect.x + 7), static_cast<int>(rect.y + (rect.height - 14) * 0.5f), 12, RAYWHITE);
}

void AnimationWindow::CommitTextField()
{
    if (activeField == TextField::ClipName) newClipName = textBuffer;
    else if (activeField == TextField::SpritePath) newSpritePath = textBuffer;
    activeField = TextField::None;
    textBuffer.clear();
}

bool AnimationWindow::AddClip()
{
    GameObject* object = scene.GetSelectedObject();
    AnimationComponent* animation = object == nullptr ? nullptr : object->GetComponent<AnimationComponent>();
    if (animation == nullptr || newSpritePath.empty() || newClipName.empty())
    {
        status = "Choose an object, clip name, and sprite sheet path.";
        return false;
    }
    if (!animation->AddAnimation(newClipName, newSpritePath, newFrameWidth, newFrameHeight,
            newFrameCount, newFPS, loopNewClip))
    {
        status = "Could not load sheet. Check path and horizontal frame dimensions.";
        return false;
    }
    animation->Play(newClipName);
    animation->Pause();
    status = "Clip added";
    addClipOpen = false;
    return true;
}

void AnimationWindow::Update()
{
    if (!visible) { consumeMouseThisFrame = false; return; }
    InputManager& input = InputManager::GetInstance();
    const Vector2 mouse = input.GetMousePosition();
    consumeMouseThisFrame = input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, bounds);
    if (dragging)
    {
        if (input.IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            bounds.x = std::clamp(mouse.x - dragOffset.x, 0.0f, std::max(0.0f, GetScreenWidth() - bounds.width));
            bounds.y = std::clamp(mouse.y - dragOffset.y, 27.0f, std::max(27.0f, GetScreenHeight() - bounds.height));
        }
        else dragging = false;
    }
    if (input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        const Rectangle title{bounds.x, bounds.y, bounds.width - 31.0f, 29.0f};
        if (CheckCollisionPointRec(mouse, CloseButton())) { visible = false; return; }
        if (CheckCollisionPointRec(mouse, title)) { dragging = true; dragOffset = {mouse.x - bounds.x, mouse.y - bounds.y}; return; }
    }

    if (activeField != TextField::None)
    {
        int character = input.GetCharPressed();
        while (character > 0)
        {
            if (character >= 32 && character <= 126) textBuffer.push_back(static_cast<char>(character));
            character = input.GetCharPressed();
        }
        if (input.IsKeyPressed(KEY_BACKSPACE) && !textBuffer.empty()) textBuffer.pop_back();
        if (input.IsKeyPressed(KEY_ENTER)) CommitTextField();
        if (input.IsKeyPressed(KEY_ESCAPE)) { activeField = TextField::None; textBuffer.clear(); }
    }

    GameObject* object = scene.GetSelectedObject();
    AnimationComponent* animation = object == nullptr ? nullptr : object->GetComponent<AnimationComponent>();
    if (previewUpdates && animation != nullptr && animation->IsEnabled() && animation->IsPlaying()) animation->Update(GetFrameTime());
    const float left = bounds.x + 12.0f;
    const float bodyTop = bounds.y + 53.0f;
    const Rectangle listRect{left, bodyTop, 178.0f, 310.0f};
    const Rectangle addButton{left, bodyTop + 316.0f, 178.0f, 26.0f};
    if (Clicked(addButton)) { addClipOpen = !addClipOpen; status.clear(); return; }

    if (animation == nullptr) return;
    const std::vector<AnimationClipInfo> clips = animation->GetAnimations();
    if (animation->GetCurrentFrame() >= timelineStart + 12) timelineStart = animation->GetCurrentFrame() - 11;
    else if (animation->GetCurrentFrame() < timelineStart) timelineStart = animation->GetCurrentFrame();
    if (CheckCollisionPointRec(mouse, listRect))
        clipScroll = std::max(0.0f, clipScroll - input.GetMouseWheelMove() * 31.0f);
    clipScroll = std::clamp(clipScroll, 0.0f,
        std::max(0.0f, static_cast<float>(clips.size()) * 31.0f - 275.0f));
    for (std::size_t index = 0; index < clips.size(); ++index)
    {
        const Rectangle row{listRect.x + 4.0f, listRect.y + 5.0f + index * 31.0f - clipScroll, listRect.width - 8.0f, 27.0f};
        if (row.y < listRect.y + 27.0f || row.y + row.height > listRect.y + listRect.height - 4.0f) continue;
        const Rectangle remove{row.x + row.width - 23.0f, row.y + 2.0f, 20.0f, 23.0f};
        if (Clicked(remove)) { animation->RemoveAnimation(clips[index].name); return; }
        if (Clicked(row)) { animation->Play(clips[index].name); animation->Pause(); animation->SetCurrentFrame(0); return; }
    }

    if (addClipOpen)
    {
        const float x = bounds.x + 205.0f;
        const float y = bounds.y + 308.0f;
        if (Clicked({x + 90, y, 270, 27})) { activeField = TextField::ClipName; textBuffer = newClipName; return; }
        if (Clicked({x + 90, y + 34, 480, 27})) { activeField = TextField::SpritePath; textBuffer = newSpritePath; return; }
        if (Clicked({x + 90, y + 68, 34, 25})) newFrameWidth = std::max(1, newFrameWidth - 1);
        if (Clicked({x + 128, y + 68, 34, 25})) newFrameWidth = std::min(4096, newFrameWidth + 1);
        if (Clicked({x + 200, y + 68, 34, 25})) newFrameHeight = std::max(1, newFrameHeight - 1);
        if (Clicked({x + 238, y + 68, 34, 25})) newFrameHeight = std::min(4096, newFrameHeight + 1);
        if (Clicked({x + 310, y + 68, 34, 25})) newFrameCount = std::max(1, newFrameCount - 1);
        if (Clicked({x + 348, y + 68, 34, 25})) newFrameCount = std::min(10000, newFrameCount + 1);
        if (Clicked({x + 420, y + 68, 34, 25})) newFPS = std::max(1.0f, newFPS - 1.0f);
        if (Clicked({x + 458, y + 68, 34, 25})) newFPS = std::min(120.0f, newFPS + 1.0f);
        if (Clicked({x + 90, y + 102, 125, 26})) loopNewClip = !loopNewClip;
        if (Clicked({x + 440, y + 102, 100, 26})) { if (activeField != TextField::None) CommitTextField(); AddClip(); }
        return;
    }
    if (clips.empty()) return;
    const auto current = std::find_if(clips.begin(), clips.end(), [&](const AnimationClipInfo& clip) { return clip.name == animation->GetCurrentAnimationName(); });
    if (current == clips.end()) return;
    const float controlsY = bounds.y + 282.0f;
    if (Clicked({bounds.x + 205, controlsY, 64, 27})) { animation->Play(current->name); return; }
    if (Clicked({bounds.x + 274, controlsY, 64, 27})) { animation->Pause(); return; }
    if (Clicked({bounds.x + 343, controlsY, 64, 27})) { animation->Stop(); return; }
    if (Clicked({bounds.x + 412, controlsY, 70, 27})) animation->SetCurrentFrame(animation->GetCurrentFrame() - 1);
    if (Clicked({bounds.x + 485, controlsY, 70, 27})) animation->SetCurrentFrame(animation->GetCurrentFrame() + 1);
    if (Clicked({bounds.x + 564, controlsY, 105, 27})) animation->SetLooping(!animation->IsLooping());
    if (Clicked({bounds.x + 243, bounds.y + 318.0f, 30, 25})) animation->SetFPS(animation->GetFPS() - 1.0f);
    if (Clicked({bounds.x + 310, bounds.y + 318.0f, 30, 25})) animation->SetFPS(animation->GetFPS() + 1.0f);
    if (Clicked({bounds.x + 410, bounds.y + 318.0f, 30, 25})) animation->SetFrameCount(animation->GetFrameCount() - 1);
    if (Clicked({bounds.x + 480, bounds.y + 318.0f, 30, 25})) animation->SetFrameCount(animation->GetFrameCount() + 1);
    if (!addClipOpen)
    {
        if (Clicked({bounds.x + 622, bounds.y + 360, 30, 54})) timelineStart = std::max(0, timelineStart - 12);
        if (Clicked({bounds.x + 656, bounds.y + 360, 30, 54})) timelineStart = std::min(std::max(0, animation->GetFrameCount() - 12), timelineStart + 12);
        for (int slot = 0; slot < 12; ++slot)
        {
            const int frame = timelineStart + slot;
            const Rectangle cell{bounds.x + 205.0f + slot * 34.0f, bounds.y + 362.0f, 32.0f, 52.0f};
            if (frame < animation->GetFrameCount() && Clicked(cell)) { animation->Pause(); animation->SetCurrentFrame(frame); }
        }
    }
}

void AnimationWindow::Draw() const
{
    if (!visible) return;
    GameObject* object = scene.GetSelectedObject();
    AnimationComponent* animation = object == nullptr ? nullptr : object->GetComponent<AnimationComponent>();
    DrawRectangleRec(bounds, WindowBg);
    DrawRectangleLinesEx(bounds, 1.0f, Color{16, 17, 19, 255});
    DrawRectangle(static_cast<int>(bounds.x), static_cast<int>(bounds.y), static_cast<int>(bounds.width), 29, HeaderBg);
    DrawText("Animation", static_cast<int>(bounds.x + 12), static_cast<int>(bounds.y + 7), 14, RAYWHITE);
    DrawText("x", static_cast<int>(CloseButton().x + 8), static_cast<int>(CloseButton().y + 4), 15, RAYWHITE);
    DrawText(object == nullptr ? "No GameObject selected" : object->GetName().c_str(), static_cast<int>(bounds.x + 12), static_cast<int>(bounds.y + 33), 11, Color{175, 178, 182, 255});
    if (animation == nullptr)
    {
        DrawText("Select a 2D GameObject with an Animation component.", static_cast<int>(bounds.x + 20), static_cast<int>(bounds.y + 75), 14, LIGHTGRAY);
        return;
    }
    const float left = bounds.x + 12.0f, top = bounds.y + 53.0f;
    DrawRectangle(static_cast<int>(left), static_cast<int>(top), 178, 310, Color{34, 35, 38, 255});
    DrawText("Animation Clips", static_cast<int>(left + 8), static_cast<int>(top + 8), 13, RAYWHITE);
    const auto clips = animation->GetAnimations();
    for (std::size_t index = 0; index < clips.size(); ++index)
    {
        const Rectangle row{left + 4.0f, top + 27.0f + index * 31.0f - clipScroll, 170.0f, 27.0f};
        if (row.y < top + 27.0f || row.y + row.height > top + 306.0f) continue;
        const bool active = clips[index].name == animation->GetCurrentAnimationName();
        DrawRectangleRec(row, active ? Color{52, 82, 117, 255} : Color{48, 49, 52, 255});
        DrawText(clips[index].name.c_str(), static_cast<int>(row.x + 8), static_cast<int>(row.y + 7), 12, RAYWHITE);
        DrawText("x", static_cast<int>(row.x + row.width - 17), static_cast<int>(row.y + 5), 13, LIGHTGRAY);
    }
    DrawButton({left, top + 316, 178, 26}, addClipOpen ? "Cancel Add Clip" : "+ Add Clip", addClipOpen);
    if (clips.empty())
    {
        DrawText("Create a clip from a horizontal sprite sheet.", static_cast<int>(bounds.x + 205), static_cast<int>(bounds.y + 74), 13, LIGHTGRAY);
    }
    else
    {
        const Texture2D* texture = animation->GetActiveTexture();
        const Rectangle preview{bounds.x + 205, bounds.y + 53, 464, 215};
        DrawRectangleRec(preview, Color{25, 26, 29, 255});
        DrawRectangleLinesEx(preview, 1, Color{75, 77, 81, 255});
        if (texture != nullptr && texture->id != 0)
        {
            const Rectangle source = animation->GetActiveSourceRect();
            const float scale = std::min((preview.width - 28) / std::max(1.0f, source.width), (preview.height - 28) / std::max(1.0f, source.height));
            const Vector2 size{source.width * scale, source.height * scale};
            DrawTexturePro(*texture, source, {preview.x + (preview.width - size.x) * 0.5f, preview.y + (preview.height - size.y) * 0.5f, size.x, size.y}, {0, 0}, 0, WHITE);
        }
        DrawText(TextFormat("Frame %d / %d", animation->GetCurrentFrame() + 1, animation->GetFrameCount()), static_cast<int>(preview.x + 8), static_cast<int>(preview.y + preview.height - 18), 10, LIGHTGRAY);
        DrawButton({bounds.x + 205, bounds.y + 282, 64, 27}, "Play", animation->IsPlaying());
        DrawButton({bounds.x + 274, bounds.y + 282, 64, 27}, "Pause");
        DrawButton({bounds.x + 343, bounds.y + 282, 64, 27}, "Stop");
        DrawButton({bounds.x + 412, bounds.y + 282, 70, 27}, "Frame -");
        DrawButton({bounds.x + 485, bounds.y + 282, 70, 27}, "Frame +");
        DrawButton({bounds.x + 564, bounds.y + 282, 105, 27}, animation->IsLooping() ? "Looping" : "Once", animation->IsLooping());
        DrawText("FPS", static_cast<int>(bounds.x + 205), static_cast<int>(bounds.y + 324), 12, LIGHTGRAY);
        DrawButton({bounds.x + 243, bounds.y + 318, 30, 25}, "-");
        DrawText(TextFormat("%.0f", animation->GetFPS()), static_cast<int>(bounds.x + 280), static_cast<int>(bounds.y + 324), 12, WHITE);
        DrawButton({bounds.x + 310, bounds.y + 318, 30, 25}, "+");
        DrawText("Frames", static_cast<int>(bounds.x + 360), static_cast<int>(bounds.y + 324), 12, LIGHTGRAY);
        DrawButton({bounds.x + 410, bounds.y + 318, 30, 25}, "-");
        DrawText(TextFormat("%d", animation->GetFrameCount()), static_cast<int>(bounds.x + 447), static_cast<int>(bounds.y + 324), 12, WHITE);
        DrawButton({bounds.x + 480, bounds.y + 318, 30, 25}, "+");
        DrawText("Timeline", static_cast<int>(bounds.x + 205), static_cast<int>(bounds.y + 351), 10, LIGHTGRAY);
        DrawButton({bounds.x + 622, bounds.y + 360, 30, 54}, "<");
        DrawButton({bounds.x + 656, bounds.y + 360, 30, 54}, ">");
        DrawRectangle(static_cast<int>(bounds.x + 205), static_cast<int>(bounds.y + 360), 408, 54, Color{31, 32, 35, 255});
        const Texture2D* sheet = animation->GetActiveTexture();
        const int frameWidth = animation->GetFrameWidth(), frameHeight = animation->GetFrameHeight();
        for (int slot = 0; slot < 12; ++slot)
        {
            const int frame = timelineStart + slot;
            if (frame >= animation->GetFrameCount()) break;
            const Rectangle cell{bounds.x + 205.0f + slot * 34.0f, bounds.y + 362.0f, 32.0f, 52.0f};
            DrawRectangleRec(cell, frame == animation->GetCurrentFrame() ? Color{61, 105, 148, 255} : Color{52, 54, 58, 255});
            if (sheet != nullptr && sheet->id != 0 && frameWidth > 0 && frameHeight > 0)
            {
                const Rectangle source{static_cast<float>(frame * frameWidth), 0, static_cast<float>(frameWidth), static_cast<float>(frameHeight)};
                DrawTexturePro(*sheet, source, {cell.x + 3, cell.y + 2, 26, 31}, {0, 0}, 0, WHITE);
            }
            DrawText(TextFormat("%d", frame + 1), static_cast<int>(cell.x + 2), static_cast<int>(cell.y + 35), 9, WHITE);
        }
    }
    if (addClipOpen)
    {
        const float x = bounds.x + 205.0f, y = bounds.y + 308.0f;
        DrawRectangle(static_cast<int>(x), static_cast<int>(y - 4), 464, 123, Color{48, 49, 52, 255});
        DrawText("Create Sprite Animation", static_cast<int>(x + 8), static_cast<int>(y), 13, RAYWHITE);
        DrawText("Name", static_cast<int>(x + 8), static_cast<int>(y + 32), 11, LIGHTGRAY);
        DrawRectangleRec({x + 90, y, 270, 27}, activeField == TextField::ClipName ? Color{75, 77, 81, 255} : FieldBg);
        DrawText((activeField == TextField::ClipName ? textBuffer : newClipName).c_str(), static_cast<int>(x + 96), static_cast<int>(y + 8), 11, WHITE);
        DrawText("Sprite Sheet Path", static_cast<int>(x + 8), static_cast<int>(y + 48), 11, LIGHTGRAY);
        DrawRectangleRec({x + 90, y + 34, 480, 27}, activeField == TextField::SpritePath ? Color{75, 77, 81, 255} : FieldBg);
        DrawText((activeField == TextField::SpritePath ? textBuffer : newSpritePath).c_str(), static_cast<int>(x + 96), static_cast<int>(y + 42), 11, WHITE);
        DrawText(TextFormat("Frame W %d", newFrameWidth), static_cast<int>(x + 8), static_cast<int>(y + 75), 10, LIGHTGRAY);
        DrawButton({x + 90, y + 68, 34, 25}, "-"); DrawButton({x + 128, y + 68, 34, 25}, "+");
        DrawText(TextFormat("H %d", newFrameHeight), static_cast<int>(x + 170), static_cast<int>(y + 75), 10, LIGHTGRAY);
        DrawButton({x + 200, y + 68, 34, 25}, "-"); DrawButton({x + 238, y + 68, 34, 25}, "+");
        DrawText(TextFormat("Frames %d", newFrameCount), static_cast<int>(x + 278), static_cast<int>(y + 75), 10, LIGHTGRAY);
        DrawButton({x + 310, y + 68, 34, 25}, "-"); DrawButton({x + 348, y + 68, 34, 25}, "+");
        DrawText(TextFormat("FPS %.0f", newFPS), static_cast<int>(x + 392), static_cast<int>(y + 75), 10, LIGHTGRAY);
        DrawButton({x + 420, y + 68, 34, 25}, "-"); DrawButton({x + 458, y + 68, 34, 25}, "+");
        DrawButton({x + 90, y + 102, 125, 26}, loopNewClip ? "Loop: On" : "Loop: Off", loopNewClip);
        DrawButton({x + 440, y + 102, 100, 26}, "Create", true);
    }
    if (!status.empty()) DrawText(status.c_str(), static_cast<int>(bounds.x + 205), static_cast<int>(bounds.y + bounds.height - 19), 10, GOLD);
}
