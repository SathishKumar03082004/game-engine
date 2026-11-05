#include "AnimationController.h"
#include <memory>
#include <algorithm>

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
        const bool exists = std::any_of(transitions.begin(), transitions.end(), [&](const Transition& transition)
        {
            return transition.from == from && transition.to == to && transition.parameter == parameter &&
                transition.expectedValue == expectedValue && transition.condition == ConditionType::Bool;
        });
        if (exists) return;
        boolParameters.emplace(parameter, false);
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
        const bool exists = std::any_of(transitions.begin(), transitions.end(), [&](const Transition& transition)
        {
            return transition.from == from && transition.to == to && transition.parameter == trigger && transition.condition == ConditionType::Trigger;
        });
        if (exists) return;
        triggerParameters.insert(trigger);
        transitions.push_back({from, to, trigger, true, ConditionType::Trigger});
    }
}

void AnimationController::AddCompletionTransition(const std::string& from, const std::string& to)
{
    if (!from.empty() && !to.empty())
    {
        const bool exists = std::any_of(transitions.begin(), transitions.end(), [&](const Transition& transition)
        {
            return transition.from == from && transition.to == to && transition.condition == ConditionType::Completed;
        });
        if (exists) return;
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

void AnimationController::AddBoolParameter(const std::string& parameter, bool value)
{
    if (!parameter.empty()) boolParameters.emplace(parameter, value);
}

bool AnimationController::RemoveBoolParameter(const std::string& parameter)
{
    if (boolParameters.erase(parameter) == 0) return false;
    transitions.erase(std::remove_if(transitions.begin(), transitions.end(), [&](const Transition& transition)
    {
        return transition.condition == ConditionType::Bool && transition.parameter == parameter;
    }), transitions.end());
    return true;
}

std::vector<std::pair<std::string, bool>> AnimationController::GetBoolParameters() const
{
    std::vector<std::pair<std::string, bool>> result(boolParameters.begin(), boolParameters.end());
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    return result;
}

void AnimationController::AddTriggerParameter(const std::string& parameter)
{
    if (!parameter.empty()) triggerParameters.insert(parameter);
}

bool AnimationController::RemoveTriggerParameter(const std::string& parameter)
{
    const bool removed = triggerParameters.erase(parameter) > 0;
    triggers.erase(parameter);
    transitions.erase(std::remove_if(transitions.begin(), transitions.end(), [&](const Transition& transition)
    {
        return transition.condition == ConditionType::Trigger && transition.parameter == parameter;
    }), transitions.end());
    return removed;
}

std::vector<std::string> AnimationController::GetTriggerParameters() const
{
    std::vector<std::string> result(triggerParameters.begin(), triggerParameters.end());
    std::sort(result.begin(), result.end());
    return result;
}

std::vector<AnimationTransitionInfo> AnimationController::GetTransitions() const
{
    std::vector<AnimationTransitionInfo> result;
    result.reserve(transitions.size());
    for (const Transition& transition : transitions)
    {
        AnimationTransitionInfo::Condition condition = AnimationTransitionInfo::Condition::Bool;
        if (transition.condition == ConditionType::Trigger) condition = AnimationTransitionInfo::Condition::Trigger;
        else if (transition.condition == ConditionType::Completed) condition = AnimationTransitionInfo::Condition::OnAnimationEnd;
        result.push_back({transition.from, transition.to, transition.parameter, transition.expectedValue, condition});
    }
    return result;
}

bool AnimationController::RemoveTransition(std::size_t index)
{
    if (index >= transitions.size()) return false;
    transitions.erase(transitions.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

bool AnimationController::SetTransitionExpectedValue(std::size_t index, bool value)
{
    if (index >= transitions.size() || transitions[index].condition != ConditionType::Bool) return false;
    transitions[index].expectedValue = value;
    return true;
}

void AnimationController::SetTrigger(const std::string& trigger)
{
    if (!trigger.empty()) { triggerParameters.insert(trigger); triggers.insert(trigger); }
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

std::unique_ptr<Component> AnimationController::Clone() const
{
    auto clone = std::make_unique<AnimationController>();
    clone->defaultState = defaultState;
    clone->currentState = currentState;
    clone->boolParameters = boolParameters;
    clone->triggers = triggers;
    clone->triggerParameters = triggerParameters;
    clone->transitions = transitions;
    return clone;
}
