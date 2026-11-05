#include "AnimatorWindow.h"
#include "../../Engine/Animation/AnimationComponent.h"
#include "../../Engine/Animation/AnimationController.h"
#include "../../Engine/Core/InputManager.h"
#include <algorithm>

namespace
{
    constexpr Color WindowBg{42, 43, 46, 255};
    constexpr Color HeaderBg{55, 57, 61, 255};
    bool Clicked(Rectangle rectangle)
    {
        return InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(InputManager::GetInstance().GetMousePosition(), rectangle);
    }
    void Button(Rectangle rect, const char* label, bool selected = false)
    {
        DrawRectangleRec(rect, selected ? Color{57, 104, 151, 255} : Color{66, 68, 72, 255});
        DrawRectangleLinesEx(rect, 1.0f, Color{23, 24, 26, 255});
        DrawText(label, static_cast<int>(rect.x + 7), static_cast<int>(rect.y + 6), 11, RAYWHITE);
    }
}

AnimatorWindow::AnimatorWindow(Scene& sceneReference) : scene(sceneReference)
{
    bounds = {std::max(16.0f, (GetScreenWidth() - 820.0f) * 0.5f), 75.0f, 820.0f, 535.0f};
}

Rectangle AnimatorWindow::CloseButton() const { return {bounds.x + bounds.width - 29.0f, bounds.y + 3.0f, 25.0f, 23.0f}; }

bool AnimatorWindow::IsMouseOver() const
{
    return consumeMouseThisFrame || (visible && CheckCollisionPointRec(InputManager::GetInstance().GetMousePosition(), bounds));
}

Rectangle AnimatorWindow::StateRect(std::size_t index) const
{
    const float x = bounds.x + 220.0f + static_cast<float>(index % 3) * 180.0f;
    const float y = bounds.y + 108.0f + static_cast<float>(index / 3) * 90.0f - graphScroll;
    return {x, y, 150.0f, 54.0f};
}

const char* AnimatorWindow::ModeLabel() const
{
    switch (linkMode)
    {
        case LinkMode::Select: return "Select State";
        case LinkMode::Completion: return "On Animation End";
        case LinkMode::Bool: return "Bool Condition";
        case LinkMode::Trigger: return "Trigger Condition";
    }
    return "Select State";
}

void AnimatorWindow::AddParameter()
{
    GameObject* object = scene.GetSelectedObject();
    AnimationController* controller = object == nullptr ? nullptr : object->GetComponent<AnimationController>();
    if (controller == nullptr || parameterBuffer.empty()) return;
    if (addAsTrigger) controller->AddTriggerParameter(parameterBuffer);
    else controller->AddBoolParameter(parameterBuffer, false);
    parameterName = parameterBuffer;
    parameterBuffer.clear();
    parameterFieldFocused = false;
}

void AnimatorWindow::Update()
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
    if (parameterFieldFocused)
    {
        int character = input.GetCharPressed();
        while (character > 0)
        {
            if (character >= 32 && character <= 126) parameterBuffer.push_back(static_cast<char>(character));
            character = input.GetCharPressed();
        }
        if (input.IsKeyPressed(KEY_BACKSPACE) && !parameterBuffer.empty()) parameterBuffer.pop_back();
        if (input.IsKeyPressed(KEY_ENTER)) AddParameter();
        if (input.IsKeyPressed(KEY_ESCAPE)) { parameterFieldFocused = false; parameterBuffer.clear(); }
    }

    GameObject* object = scene.GetSelectedObject();
    if (object == nullptr) return;
    AnimationController* controller = object->GetComponent<AnimationController>();
    AnimationComponent* animation = object->GetComponent<AnimationComponent>();
    if (controller == nullptr)
    {
        if (Clicked({bounds.x + 20, bounds.y + 78, 190, 30}))
        {
            object->AddComponent<AnimationController>();
            status = "Animator Controller added";
        }
        return;
    }
    if (animation != nullptr && animation->IsEnabled() && animation->IsPlaying())
    {
        controller->Update(GetFrameTime());
        animation->Update(GetFrameTime());
    }

    if (Clicked({bounds.x + 215, bounds.y + 40, 135, 27})) { linkMode = LinkMode::Select; sourceState.clear(); }
    if (Clicked({bounds.x + 355, bounds.y + 40, 145, 27})) { linkMode = LinkMode::Completion; sourceState.clear(); }
    if (Clicked({bounds.x + 505, bounds.y + 40, 125, 27})) { linkMode = LinkMode::Bool; sourceState.clear(); }
    if (Clicked({bounds.x + 635, bounds.y + 40, 145, 27})) { linkMode = LinkMode::Trigger; sourceState.clear(); }

    const Rectangle paramField{bounds.x + 14, bounds.y + 94, 128, 25};
    if (Clicked(paramField)) parameterFieldFocused = true;
    if (Clicked({bounds.x + 146, bounds.y + 94, 30, 25})) addAsTrigger = !addAsTrigger;
    if (Clicked({bounds.x + 144, bounds.y + 124, 34, 25})) AddParameter();

    const auto bools = controller->GetBoolParameters();
    const auto triggers = controller->GetTriggerParameters();
    float parameterY = bounds.y + 157.0f;
    for (const auto& parameter : bools)
    {
        const Rectangle row{bounds.x + 12, parameterY, 166, 28};
        if (Clicked(row)) { parameterName = parameter.first; controller->SetBool(parameter.first, !parameter.second); }
        parameterY += 31.0f;
    }
    parameterY += 10.0f;
    for (const std::string& trigger : triggers)
    {
        const Rectangle row{bounds.x + 12, parameterY, 166, 28};
        if (Clicked(row)) { parameterName = trigger; controller->SetTrigger(trigger); }
        parameterY += 31.0f;
    }

    if (animation == nullptr) return;
    const auto clips = animation->GetAnimations();
    if (mouse.x >= bounds.x + 200.0f && mouse.y >= bounds.y + 70.0f && input.GetMouseWheelMove() != 0.0f)
        graphScroll = std::max(0.0f, graphScroll - input.GetMouseWheelMove() * 70.0f);
    const float graphContentHeight = static_cast<float>((clips.size() + 2) / 3) * 90.0f;
    graphScroll = std::clamp(graphScroll, 0.0f, std::max(0.0f, graphContentHeight - (bounds.height - 135.0f)));
    for (std::size_t index = 0; index < clips.size(); ++index)
    {
        if (!Clicked(StateRect(index))) continue;
        if (linkMode == LinkMode::Select)
        {
            controller->SetDefaultState(clips[index].name);
            controller->PlayState(clips[index].name);
            status = "Entry state set to " + clips[index].name;
            return;
        }
        if (sourceState.empty())
        {
            sourceState = clips[index].name;
            status = "Choose destination state";
            return;
        }
        if (sourceState == clips[index].name) { sourceState.clear(); status = "Transition cancelled"; return; }
        if (linkMode == LinkMode::Completion) controller->AddCompletionTransition(sourceState, clips[index].name);
        else if (linkMode == LinkMode::Bool)
        {
            if (parameterName.empty()) { status = "Add/select a bool parameter first"; sourceState.clear(); return; }
            controller->AddBoolTransition(sourceState, clips[index].name, parameterName, true);
        }
        else if (linkMode == LinkMode::Trigger)
        {
            if (parameterName.empty()) { status = "Add/select a trigger parameter first"; sourceState.clear(); return; }
            controller->AddTriggerTransition(sourceState, clips[index].name, parameterName);
        }
        status = "Transition added";
        sourceState.clear();
        return;
    }
}

void AnimatorWindow::Draw() const
{
    if (!visible) return;
    GameObject* object = scene.GetSelectedObject();
    AnimationController* controller = object == nullptr ? nullptr : object->GetComponent<AnimationController>();
    AnimationComponent* animation = object == nullptr ? nullptr : object->GetComponent<AnimationComponent>();
    DrawRectangleRec(bounds, WindowBg);
    DrawRectangleLinesEx(bounds, 1, Color{16, 17, 19, 255});
    DrawRectangle(static_cast<int>(bounds.x), static_cast<int>(bounds.y), static_cast<int>(bounds.width), 29, HeaderBg);
    DrawText("Animator", static_cast<int>(bounds.x + 12), static_cast<int>(bounds.y + 7), 14, RAYWHITE);
    DrawText("x", static_cast<int>(CloseButton().x + 8), static_cast<int>(CloseButton().y + 4), 15, RAYWHITE);
    DrawText(object == nullptr ? "No GameObject selected" : object->GetName().c_str(), static_cast<int>(bounds.x + 14), static_cast<int>(bounds.y + 34), 11, LIGHTGRAY);
    if (controller == nullptr)
    {
        DrawText("This GameObject has no Animator Controller component.", static_cast<int>(bounds.x + 20), static_cast<int>(bounds.y + 55), 14, LIGHTGRAY);
        Button({bounds.x + 20, bounds.y + 78, 190, 30}, "+ Add Animator Controller", true);
        return;
    }
    Button({bounds.x + 215, bounds.y + 40, 135, 27}, "Select", linkMode == LinkMode::Select);
    Button({bounds.x + 355, bounds.y + 40, 145, 27}, "On Animation End", linkMode == LinkMode::Completion);
    Button({bounds.x + 505, bounds.y + 40, 125, 27}, "Bool", linkMode == LinkMode::Bool);
    Button({bounds.x + 635, bounds.y + 40, 145, 27}, "Trigger", linkMode == LinkMode::Trigger);
    DrawRectangle(static_cast<int>(bounds.x + 8), static_cast<int>(bounds.y + 72), 180, static_cast<int>(bounds.height - 84), Color{34, 35, 38, 255});
    DrawText("Parameters", static_cast<int>(bounds.x + 16), static_cast<int>(bounds.y + 78), 13, RAYWHITE);
    DrawRectangleRec({bounds.x + 14, bounds.y + 94, 128, 25}, parameterFieldFocused ? Color{78, 80, 84, 255} : Color{26, 27, 29, 255});
    DrawText((parameterFieldFocused ? parameterBuffer : "Parameter name").c_str(), static_cast<int>(bounds.x + 19), static_cast<int>(bounds.y + 101), 10, LIGHTGRAY);
    Button({bounds.x + 146, bounds.y + 94, 30, 25}, addAsTrigger ? "Tr" : "B", addAsTrigger);
    Button({bounds.x + 144, bounds.y + 124, 34, 25}, "+", true);
    float parameterY = bounds.y + 157.0f;
    for (const auto& parameter : controller->GetBoolParameters())
    {
        const Rectangle row{bounds.x + 12, parameterY, 166, 28};
        DrawRectangleRec(row, Color{51, 52, 56, 255});
        DrawText(parameter.first.c_str(), static_cast<int>(row.x + 7), static_cast<int>(row.y + 8), 11, RAYWHITE);
        DrawRectangleRec({row.x + row.width - 41, row.y + 4, 35, 20}, parameter.second ? DARKGREEN : DARKGRAY);
        DrawText(parameter.second ? "True" : "False", static_cast<int>(row.x + row.width - 38), static_cast<int>(row.y + 9), 9, WHITE);
        parameterY += 31.0f;
    }
    parameterY += 10.0f;
    for (const std::string& trigger : controller->GetTriggerParameters())
    {
        const Rectangle row{bounds.x + 12, parameterY, 166, 28};
        DrawRectangleRec(row, Color{51, 52, 56, 255});
        DrawText(trigger.c_str(), static_cast<int>(row.x + 7), static_cast<int>(row.y + 8), 11, RAYWHITE);
        DrawText("Fire", static_cast<int>(row.x + row.width - 34), static_cast<int>(row.y + 8), 10, GOLD);
        parameterY += 31.0f;
    }
    DrawText(TextFormat("Current: %s", controller->GetCurrentState().empty() ? "None" : controller->GetCurrentState().c_str()), static_cast<int>(bounds.x + 15), static_cast<int>(bounds.y + bounds.height - 20), 10, LIGHTGRAY);
    if (animation == nullptr || animation->GetAnimations().empty())
    {
        DrawText("Add named sprite clips in the Animation window to create states.", static_cast<int>(bounds.x + 220), static_cast<int>(bounds.y + 100), 13, LIGHTGRAY);
        return;
    }
    const auto clips = animation->GetAnimations();
    const auto transitions = controller->GetTransitions();
    for (const AnimationTransitionInfo& transition : transitions)
    {
        auto from = std::find_if(clips.begin(), clips.end(), [&](const AnimationClipInfo& clip) { return clip.name == transition.from; });
        auto to = std::find_if(clips.begin(), clips.end(), [&](const AnimationClipInfo& clip) { return clip.name == transition.to; });
        if (from == clips.end() || to == clips.end()) continue;
        const std::size_t a = static_cast<std::size_t>(from - clips.begin()), b = static_cast<std::size_t>(to - clips.begin());
        const Rectangle ra = StateRect(a), rb = StateRect(b);
        DrawLineEx({ra.x + ra.width * 0.5f, ra.y + ra.height * 0.5f}, {rb.x + rb.width * 0.5f, rb.y + rb.height * 0.5f}, 2, Color{138, 157, 178, 255});
    }
    for (std::size_t index = 0; index < clips.size(); ++index)
    {
        const Rectangle node = StateRect(index);
        if (node.y + node.height < bounds.y + 72 || node.y > bounds.y + bounds.height - 32) continue;
        const bool current = controller->GetCurrentState() == clips[index].name;
        const bool entry = controller->GetDefaultState() == clips[index].name;
        DrawRectangleRec(node, current ? Color{50, 95, 65, 255} : Color{57, 60, 65, 255});
        DrawRectangleLinesEx(node, entry ? 3.0f : 1.0f, entry ? Color{94, 169, 224, 255} : Color{25, 26, 29, 255});
        DrawText(clips[index].name.c_str(), static_cast<int>(node.x + 10), static_cast<int>(node.y + 9), 13, RAYWHITE);
        DrawText(entry ? "Entry State" : (current ? "Playing" : "Animation State"), static_cast<int>(node.x + 10), static_cast<int>(node.y + 31), 10, LIGHTGRAY);
    }
    if (!status.empty()) DrawText(status.c_str(), static_cast<int>(bounds.x + 220), static_cast<int>(bounds.y + bounds.height - 20), 10, GOLD);
    else if (linkMode != LinkMode::Select) DrawText(sourceState.empty() ? "Select a source state, then a destination." : TextFormat("Source: %s — choose destination", sourceState.c_str()), static_cast<int>(bounds.x + 220), static_cast<int>(bounds.y + bounds.height - 20), 10, GOLD);
}
