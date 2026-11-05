#include "PlayModeToolbar.h"
#include "../../Engine/Core/InputManager.h"

Rectangle PlayModeToolbar::GetButton(int index) const
{
    const float width = 54.0f;
    const float gap = 3.0f;
    const float total = width * 4.0f + gap * 3.0f;
    return {GetScreenWidth() * 0.5f - total * 0.5f + index * (width + gap), 30.0f, width, 25.0f};
}
PlayModeToolbar::Action PlayModeToolbar::Update(State state)
{
    InputManager& input = InputManager::GetInstance();
    if (!input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return Action::None;
    for (int i = 0; i < 4; ++i)
    {
        if (!CheckCollisionPointRec(input.GetMousePosition(), GetButton(i))) continue;
        if (i == 0) return state == State::Stopped ? Action::Play : (state == State::Paused ? Action::Play : Action::None);
        if (i == 1) return state == State::Playing ? Action::Pause : Action::None;
        if (i == 2) return state == State::Paused ? Action::Step : Action::None;
        if (i == 3) return state == State::Stopped ? Action::None : Action::Stop;
    }
    return Action::None;
}
void PlayModeToolbar::Draw(State state) const
{
    const char* labels[] = {state == State::Paused ? "Resume" : "Play", "Pause", "Step", "Stop"};
    for (int i = 0; i < 4; ++i)
    {
        const Rectangle button = GetButton(i);
        const bool enabled = i == 0 || i == 3 || (i == 1 && state == State::Playing) || (i == 2 && state == State::Paused);
        const bool hover = CheckCollisionPointRec(GetMousePosition(), button);
        DrawRectangleRec(button, !enabled ? Color{60,60,60,255} : hover ? SKYBLUE : DARKGRAY);
        DrawRectangleLinesEx(button, 1, LIGHTGRAY);
        const int textWidth = MeasureText(labels[i], 11);
        DrawText(labels[i], static_cast<int>(button.x + (button.width - textWidth) * 0.5f), static_cast<int>(button.y + 7), 11, enabled ? WHITE : GRAY);
    }
    if (state != State::Stopped)
    {
        const char* label = state == State::Playing ? "PLAYING" : "PAUSED";
        DrawText(label, static_cast<int>(GetScreenWidth() * 0.5f + 122), 37, 11, state == State::Playing ? GREEN : YELLOW);
    }
}
bool PlayModeToolbar::IsMouseOver() const
{
    const float total = 54.0f * 4.0f + 3.0f * 3.0f;
    return CheckCollisionPointRec(InputManager::GetInstance().GetMousePosition(),
        {GetScreenWidth() * 0.5f - total * 0.5f, 30, total, 25});
}
