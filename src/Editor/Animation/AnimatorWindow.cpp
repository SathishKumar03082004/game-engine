#include "AnimatorWindow.h"
#include "../../Engine/Animation/AnimationComponent.h"
#include "../../Engine/Animation/AnimationController.h"
#include "../../Engine/Core/InputManager.h"
#include <algorithm>
#include <cmath>

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
    bounds = {std::max(0.0f, static_cast<float>(GetScreenWidth()) - 840.0f),
        std::max(27.0f, static_cast<float>(GetScreenHeight()) - 560.0f), 820.0f, 535.0f};
}

Rectangle AnimatorWindow::CloseButton() const { return {bounds.x + bounds.width - 29.0f, bounds.y + 3.0f, 25.0f, 23.0f}; }

bool AnimatorWindow::IsMouseOver() const
{
    return consumeMouseThisFrame || (visible && CheckCollisionPointRec(InputManager::GetInstance().GetMousePosition(), bounds));
}

Rectangle AnimatorWindow::StateRect(std::size_t index, const std::string& state) const
{
    const auto found = statePositions.find(state);
    if (found != statePositions.end()) return {found->second.x, found->second.y - graphScroll, 150.0f, 54.0f};
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
        case LinkMode::Bool: return expectedBoolValue ? "Bool == True" : "Bool == False";
        case LinkMode::Trigger: return "Trigger Condition";
    }
    return "Select State";
}

void AnimatorWindow::ActivateState(AnimationController& controller, const std::string& state)
{
    if (linkMode == LinkMode::Select)
    {
        controller.SetDefaultState(state);
        controller.PlayState(state);
        status = "Entry state set to " + state;
        return;
    }
    if (sourceState.empty())
    {
        sourceState = state;
        status = "Choose destination state";
        return;
    }
    if (sourceState == state)
    {
        sourceState.clear();
        status = "Transition cancelled";
        return;
    }
    if (linkMode == LinkMode::Completion) controller.AddCompletionTransition(sourceState, state);
    else if (linkMode == LinkMode::Bool)
    {
        if (parameterName.empty()) { status = "Add/select a bool parameter first"; sourceState.clear(); return; }
        controller.AddBoolTransition(sourceState, state, parameterName, expectedBoolValue);
    }
    else if (linkMode == LinkMode::Trigger)
    {
        if (parameterName.empty()) { status = "Add/select a trigger parameter first"; sourceState.clear(); return; }
        controller.AddTriggerTransition(sourceState, state, parameterName);
    }
    selectedTransition = static_cast<int>(controller.GetTransitions().size()) - 1;
    transitionScroll = std::max(0, selectedTransition - 3);
    status = "Transition added";
    sourceState.clear();
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
    if (positionsOwner != object)
    {
        positionsOwner = object;
        statePositions.clear();
        sourceState.clear();
        parameterName.clear();
        selectedTransition = -1;
        graphScroll = 0.0f;
        parameterScroll = 0.0f;
    }
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
    if (animation != nullptr && animation->IsEnabled())
    {
        controller->Update(GetFrameTime());
        if (animation->IsPlaying()) animation->Update(GetFrameTime());
    }

    if (Clicked({bounds.x + 215, bounds.y + 40, 135, 27})) { linkMode = LinkMode::Select; sourceState.clear(); status.clear(); }
    if (Clicked({bounds.x + 355, bounds.y + 40, 145, 27})) { linkMode = LinkMode::Completion; sourceState.clear(); status.clear(); }
    if (Clicked({bounds.x + 505, bounds.y + 40, 125, 27}))
    {
        if (linkMode == LinkMode::Bool) expectedBoolValue = !expectedBoolValue;
        else linkMode = LinkMode::Bool;
        sourceState.clear(); status.clear();
    }
    if (Clicked({bounds.x + 635, bounds.y + 40, 145, 27})) { linkMode = LinkMode::Trigger; sourceState.clear(); status.clear(); }

    const Rectangle paramField{bounds.x + 14, bounds.y + 94, 128, 25};
    if (Clicked(paramField)) parameterFieldFocused = true;
    if (Clicked({bounds.x + 146, bounds.y + 94, 30, 25})) addAsTrigger = !addAsTrigger;
    if (Clicked({bounds.x + 144, bounds.y + 124, 34, 25})) AddParameter();

    const auto bools = controller->GetBoolParameters();
    const auto triggers = controller->GetTriggerParameters();
    const float transitionTop = bounds.y + bounds.height - 90.0f;
    const float parameterViewport = std::max(0.0f, transitionTop - (bounds.y + 157.0f));
    const float parameterContent = static_cast<float>(bools.size() + triggers.size()) * 31.0f + (triggers.empty() || bools.empty() ? 0.0f : 10.0f);
    if (mouse.x < bounds.x + 190.0f && mouse.y >= bounds.y + 150.0f && mouse.y < transitionTop && input.GetMouseWheelMove() != 0.0f)
        parameterScroll = std::max(0.0f, parameterScroll - input.GetMouseWheelMove() * 31.0f);
    parameterScroll = std::clamp(parameterScroll, 0.0f, std::max(0.0f, parameterContent - parameterViewport));
    float parameterY = bounds.y + 157.0f - parameterScroll;
    for (const auto& parameter : bools)
    {
        const Rectangle row{bounds.x + 12, parameterY, 166, 28};
        if (row.y >= bounds.y + 150.0f && row.y + row.height < transitionTop && Clicked(row))
        {
            if (mouse.x > row.x + row.width - 23.0f)
            {
                controller->RemoveBoolParameter(parameter.first);
                if (parameterName == parameter.first) parameterName.clear();
                selectedTransition = -1;
            }
            else if (mouse.x > row.x + row.width - 53.0f) controller->SetBool(parameter.first, !parameter.second);
            else parameterName = parameter.first;
        }
        parameterY += 31.0f;
    }
    if (!bools.empty() && !triggers.empty()) parameterY += 10.0f;
    for (const std::string& trigger : triggers)
    {
        const Rectangle row{bounds.x + 12, parameterY, 166, 28};
        if (row.y >= bounds.y + 150.0f && row.y + row.height < transitionTop && Clicked(row))
        {
            if (mouse.x > row.x + row.width - 23.0f)
            {
                controller->RemoveTriggerParameter(trigger);
                if (parameterName == trigger) parameterName.clear();
                selectedTransition = -1;
            }
            else if (mouse.x > row.x + row.width - 50.0f) controller->SetTrigger(trigger);
            else parameterName = trigger;
        }
        parameterY += 31.0f;
    }

    if (animation == nullptr) return;
    const auto clips = animation->GetAnimations();
    if (mouse.x >= bounds.x + 200.0f && mouse.y >= bounds.y + 70.0f && mouse.y < transitionTop && input.GetMouseWheelMove() != 0.0f)
        graphScroll = std::max(0.0f, graphScroll - input.GetMouseWheelMove() * 70.0f);
    const float graphContentHeight = static_cast<float>((clips.size() + 2) / 3) * 90.0f;
    graphScroll = std::clamp(graphScroll, 0.0f, std::max(0.0f, graphContentHeight - (transitionTop - (bounds.y + 108.0f))));

    auto transitions = controller->GetTransitions();
    const int visibleTransitionCount = 4;
    if (mouse.x >= bounds.x + 210.0f && mouse.y >= transitionTop && input.GetMouseWheelMove() != 0.0f)
        transitionScroll = std::clamp(transitionScroll - static_cast<int>(input.GetMouseWheelMove()), 0,
            std::max(0, static_cast<int>(transitions.size()) - visibleTransitionCount));
    for (int visibleIndex = 0; visibleIndex < visibleTransitionCount; ++visibleIndex)
    {
        const int transitionIndex = transitionScroll + visibleIndex;
        if (transitionIndex >= static_cast<int>(transitions.size())) break;
        const Rectangle row{bounds.x + 220.0f + visibleIndex * 140.0f, transitionTop + 18.0f, 134.0f, 23.0f};
        if (Clicked(row)) { selectedTransition = transitionIndex; status.clear(); }
    }
    if (selectedTransition >= static_cast<int>(transitions.size())) selectedTransition = static_cast<int>(transitions.size()) - 1;
    if (selectedTransition >= 0 && selectedTransition < static_cast<int>(transitions.size()))
    {
        const auto& transition = transitions[static_cast<std::size_t>(selectedTransition)];
        const Rectangle details{bounds.x + 220.0f, transitionTop + 45.0f, 400.0f, 27.0f};
        if (transition.condition == AnimationTransitionInfo::Condition::Bool && Clicked({details.x + 300.0f, details.y, 62.0f, details.height}))
            controller->SetTransitionExpectedValue(static_cast<std::size_t>(selectedTransition), !transition.expectedValue);
        if (Clicked({details.x + 368.0f, details.y, 30.0f, details.height}))
        {
            controller->RemoveTransition(static_cast<std::size_t>(selectedTransition));
            selectedTransition = -1;
            status = "Transition removed";
            return;
        }
    }

    if (!draggingState.empty())
    {
        if (input.IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            const Vector2 delta{mouse.x - stateDragStart.x, mouse.y - stateDragStart.y};
            if (std::fabs(delta.x) + std::fabs(delta.y) > 4.0f) stateWasDragged = true;
            statePositions[draggingState] = {
                std::clamp(mouse.x - dragOffset.x, bounds.x + 205.0f, bounds.x + bounds.width - 160.0f),
                std::clamp(mouse.y - dragOffset.y + graphScroll, bounds.y + 82.0f + graphScroll,
                    transitionTop - 64.0f + graphScroll)};
            return;
        }
        if (input.IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            const std::string releasedState = draggingState;
            draggingState.clear();
            if (!stateWasDragged) ActivateState(*controller, releasedState);
            stateWasDragged = false;
            return;
        }
    }
    for (std::size_t index = 0; index < clips.size(); ++index)
    {
        const Rectangle node = StateRect(index, clips[index].name);
        if (node.y < bounds.y + 72.0f || node.y + node.height > transitionTop) continue;
        if (Clicked(node))
        {
            draggingState = clips[index].name;
            dragOffset = {mouse.x - node.x, mouse.y - node.y};
            stateDragStart = mouse;
            stateWasDragged = false;
            return;
        }
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
    Button({bounds.x + 505, bounds.y + 40, 125, 27}, expectedBoolValue ? "Bool == True" : "Bool == False", linkMode == LinkMode::Bool);
    Button({bounds.x + 635, bounds.y + 40, 145, 27}, "Trigger", linkMode == LinkMode::Trigger);
    DrawRectangle(static_cast<int>(bounds.x + 8), static_cast<int>(bounds.y + 72), 180, static_cast<int>(bounds.height - 84), Color{34, 35, 38, 255});
    DrawText("Parameters", static_cast<int>(bounds.x + 16), static_cast<int>(bounds.y + 78), 13, RAYWHITE);
    DrawRectangleRec({bounds.x + 14, bounds.y + 94, 128, 25}, parameterFieldFocused ? Color{78, 80, 84, 255} : Color{26, 27, 29, 255});
    DrawText((parameterFieldFocused ? parameterBuffer : "Parameter name").c_str(), static_cast<int>(bounds.x + 19), static_cast<int>(bounds.y + 101), 10, LIGHTGRAY);
    Button({bounds.x + 146, bounds.y + 94, 30, 25}, addAsTrigger ? "Tr" : "B", addAsTrigger);
    Button({bounds.x + 144, bounds.y + 124, 34, 25}, "+", true);
    const float transitionTop = bounds.y + bounds.height - 90.0f;
    float parameterY = bounds.y + 157.0f - parameterScroll;
    for (const auto& parameter : controller->GetBoolParameters())
    {
        if (parameterY < bounds.y + 150.0f || parameterY + 28.0f > transitionTop) { parameterY += 31.0f; continue; }
        const Rectangle row{bounds.x + 12, parameterY, 166, 28};
        DrawRectangleRec(row, Color{51, 52, 56, 255});
        DrawText(TextFormat("%.16s", parameter.first.c_str()), static_cast<int>(row.x + 7), static_cast<int>(row.y + 8), 11, RAYWHITE);
        DrawRectangleRec({row.x + row.width - 59, row.y + 4, 35, 20}, parameter.second ? DARKGREEN : DARKGRAY);
        DrawText(parameter.second ? "True" : "False", static_cast<int>(row.x + row.width - 56), static_cast<int>(row.y + 9), 9, WHITE);
        DrawText("x", static_cast<int>(row.x + row.width - 17), static_cast<int>(row.y + 7), 12, LIGHTGRAY);
        parameterY += 31.0f;
    }
    if (!controller->GetBoolParameters().empty() && !controller->GetTriggerParameters().empty()) parameterY += 10.0f;
    for (const std::string& trigger : controller->GetTriggerParameters())
    {
        if (parameterY < bounds.y + 150.0f || parameterY + 28.0f > transitionTop) { parameterY += 31.0f; continue; }
        const Rectangle row{bounds.x + 12, parameterY, 166, 28};
        DrawRectangleRec(row, Color{51, 52, 56, 255});
        DrawText(TextFormat("%.16s", trigger.c_str()), static_cast<int>(row.x + 7), static_cast<int>(row.y + 8), 11, RAYWHITE);
        DrawText("Fire", static_cast<int>(row.x + row.width - 48), static_cast<int>(row.y + 8), 10, GOLD);
        DrawText("x", static_cast<int>(row.x + row.width - 17), static_cast<int>(row.y + 7), 12, LIGHTGRAY);
        parameterY += 31.0f;
    }
    DrawText(TextFormat("Current: %s", controller->GetCurrentState().empty() ? "None" : controller->GetCurrentState().c_str()), static_cast<int>(bounds.x + 15), static_cast<int>(bounds.y + bounds.height - 18), 10, LIGHTGRAY);
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
        const Rectangle ra = StateRect(a, from->name), rb = StateRect(b, to->name);
        DrawLineEx({ra.x + ra.width * 0.5f, ra.y + ra.height * 0.5f}, {rb.x + rb.width * 0.5f, rb.y + rb.height * 0.5f}, 2, Color{138, 157, 178, 255});
        std::string label = transition.condition == AnimationTransitionInfo::Condition::OnAnimationEnd ? "Exit" : transition.parameter;
        if (transition.condition == AnimationTransitionInfo::Condition::Bool) label += transition.expectedValue ? " = true" : " = false";
        DrawText(label.c_str(), static_cast<int>((ra.x + rb.x) * 0.5f + 5), static_cast<int>((ra.y + rb.y) * 0.5f), 9, GOLD);
    }
    for (std::size_t index = 0; index < clips.size(); ++index)
    {
        const Rectangle node = StateRect(index, clips[index].name);
        if (node.y + node.height < bounds.y + 72 || node.y + node.height > transitionTop) continue;
        const bool current = controller->GetCurrentState() == clips[index].name;
        const bool entry = controller->GetDefaultState() == clips[index].name;
        DrawRectangleRec(node, current ? Color{50, 95, 65, 255} : Color{57, 60, 65, 255});
        DrawRectangleLinesEx(node, entry ? 3.0f : 1.0f, entry ? Color{94, 169, 224, 255} : Color{25, 26, 29, 255});
        DrawText(clips[index].name.c_str(), static_cast<int>(node.x + 10), static_cast<int>(node.y + 9), 13, RAYWHITE);
        DrawText(entry ? "Entry State" : (current ? "Playing" : "Animation State"), static_cast<int>(node.x + 10), static_cast<int>(node.y + 31), 10, LIGHTGRAY);
    }
    DrawRectangle(static_cast<int>(bounds.x + 205), static_cast<int>(transitionTop), static_cast<int>(bounds.width - 215), 82, Color{35, 36, 39, 255});
    DrawText("Transitions", static_cast<int>(bounds.x + 216), static_cast<int>(transitionTop + 4), 11, RAYWHITE);
    const auto transitionList = controller->GetTransitions();
    for (int visibleIndex = 0; visibleIndex < 4; ++visibleIndex)
    {
        const int transitionIndex = transitionScroll + visibleIndex;
        if (transitionIndex >= static_cast<int>(transitionList.size())) break;
        const auto& transition = transitionList[static_cast<std::size_t>(transitionIndex)];
        const Rectangle row{bounds.x + 220.0f + visibleIndex * 140.0f, transitionTop + 18.0f, 134.0f, 23.0f};
        const std::string label = transition.from.substr(0, 12) + " > " + transition.to.substr(0, 12);
        DrawRectangleRec(row, transitionIndex == selectedTransition ? Color{57, 104, 151, 255} : Color{60, 62, 66, 255});
        DrawText(label.c_str(), static_cast<int>(row.x + 5), static_cast<int>(row.y + 6), 9, RAYWHITE);
    }
    if (selectedTransition >= 0 && selectedTransition < static_cast<int>(transitionList.size()))
    {
        const auto& transition = transitionList[static_cast<std::size_t>(selectedTransition)];
        const Rectangle detail{bounds.x + 220.0f, transitionTop + 45.0f, 400.0f, 27.0f};
        const char* condition = transition.condition == AnimationTransitionInfo::Condition::OnAnimationEnd ? "On Animation End" :
            (transition.condition == AnimationTransitionInfo::Condition::Trigger ? "Trigger" : "Bool");
        std::string detailText = transition.from + " -> " + transition.to + " | " + condition;
        if (transition.condition == AnimationTransitionInfo::Condition::Bool)
            detailText += " | " + transition.parameter + (transition.expectedValue ? " = true" : " = false");
        else if (transition.condition == AnimationTransitionInfo::Condition::Trigger)
            detailText += " | " + transition.parameter;
        if (detailText.size() > 43) detailText = detailText.substr(0, 40) + "...";
        DrawText(detailText.c_str(), static_cast<int>(detail.x + 4), static_cast<int>(detail.y + 8), 9, LIGHTGRAY);
        if (transition.condition == AnimationTransitionInfo::Condition::Bool)
        {
            const Rectangle expected{detail.x + 300, detail.y, 62, detail.height};
            DrawRectangleRec(expected, transition.expectedValue ? DARKGREEN : MAROON);
            DrawText(transition.expectedValue ? "True" : "False", static_cast<int>(expected.x + 14), static_cast<int>(expected.y + 8), 9, WHITE);
        }
        Button({detail.x + 368, detail.y, 30, detail.height}, "x");
    }
    const char* hint = !status.empty() ? status.c_str() : (linkMode == LinkMode::Select
        ? "Drag states to arrange. Click a state to set the entry state."
        : (sourceState.empty() ? "Select a source state, then a destination." : TextFormat("Source: %s — choose destination", sourceState.c_str())));
    DrawText(hint, static_cast<int>(bounds.x + 220), static_cast<int>(bounds.y + bounds.height - 17), 9, GOLD);
}
