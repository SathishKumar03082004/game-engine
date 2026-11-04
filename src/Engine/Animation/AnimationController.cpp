#include "AnimationController.h"

#include "AnimationComponent.h"
#include "../Scene/GameObject.h"

void AnimationController::SetDefaultState(const std::string& state)
{
    defaultState = state;
    if (currentState.empty()) currentState = state;
}

const std::string& AnimationController::GetDefaultState() const
{
    return defaultState;
}

const std::string& AnimationController::GetCurrentState() const
{
    return currentState;
}

bool AnimationController::PlayState(const std::string& state)
{
    return TransitionTo(state);
}

void AnimationController::AddBoolTransition(
    const std::string& from,
    const std::string& to,
    const std::string& parameter,
    bool expectedValue
)
{
    if (!from.empty() && !to.empty() && !parameter.empty())
    {
        transitions.push_back({from, to, parameter, expectedValue, ConditionType::Bool});
    }
}

void AnimationController::AddTriggerTransition(
    const std::string& from,
    const std::string& to,
    const std::string& trigger
)
{
    if (!from.empty() && !to.empty() && !trigger.empty())
    {
        transitions.push_back({from, to, trigger, true, ConditionType::Trigger});
    }
}

void AnimationController::AddCompletionTransition(const std::string& from, const std::string& to)
{
    if (!from.empty() && !to.empty())
    {
        transitions.push_back({from, to, {}, true, ConditionType::Completed});
    }
}

void AnimationController::SetBool(const std::string& parameter, bool value)
{
    boolParameters[parameter] = value;
}

bool AnimationController::GetBool(const std::string& parameter) const
{
    const auto found = boolParameters.find(parameter);
    return found != boolParameters.end() && found->second;
}

void AnimationController::SetTrigger(const std::string& trigger)
{
    if (!trigger.empty()) triggers.insert(trigger);
}

void AnimationController::ResetTrigger(const std::string& trigger)
{
    triggers.erase(trigger);
}

void AnimationController::OnStart()
{
    if (!defaultState.empty()) TransitionTo(defaultState);
}

void AnimationController::Update(float deltaTime)
{
    static_cast<void>(deltaTime);
    AnimationComponent* animation = GetOwner() == nullptr
        ? nullptr
        : GetOwner()->GetComponent<AnimationComponent>();
    if (animation == nullptr) return;

    for (const Transition& transition : transitions)
    {
        if (transition.from != currentState) continue;

        bool matches = false;
        switch (transition.condition)
        {
            case ConditionType::Bool:
                matches = GetBool(transition.parameter) == transition.expectedValue;
                break;
            case ConditionType::Trigger:
                matches = triggers.find(transition.parameter) != triggers.end();
                break;
            case ConditionType::Completed:
                matches = animation->GetCurrentAnimationName() == currentState && animation->IsFinished();
                break;
        }

        if (matches)
        {
            if (transition.condition == ConditionType::Trigger) ResetTrigger(transition.parameter);
            TransitionTo(transition.to);
            return;
        }
    }
}

bool AnimationController::TransitionTo(const std::string& state)
{
    if (GetOwner() == nullptr) return false;
    AnimationComponent* animation = GetOwner()->GetComponent<AnimationComponent>();
    if (animation == nullptr || !animation->Play(state)) return false;
    currentState = state;
    return true;
}
