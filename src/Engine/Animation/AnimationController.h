#pragma once

#include "../Core/Component.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct AnimationTransitionInfo
{
    enum class Condition { Bool, Trigger, OnAnimationEnd };
    std::string from;
    std::string to;
    std::string parameter;
    bool expectedValue = true;
    Condition condition = Condition::Bool;
};

class AnimationController final : public Component
{
public:
    void SetDefaultState(const std::string& state);
    const std::string& GetDefaultState() const;
    const std::string& GetCurrentState() const;
    bool PlayState(const std::string& state);

    void AddBoolTransition(
        const std::string& from,
        const std::string& to,
        const std::string& parameter,
        bool expectedValue
    );
    void AddTriggerTransition(
        const std::string& from,
        const std::string& to,
        const std::string& trigger
    );
    void AddCompletionTransition(const std::string& from, const std::string& to);

    void SetBool(const std::string& parameter, bool value);
    bool GetBool(const std::string& parameter) const;
    void AddBoolParameter(const std::string& parameter, bool value = false);
    bool RemoveBoolParameter(const std::string& parameter);
    std::vector<std::pair<std::string, bool>> GetBoolParameters() const;
    void AddTriggerParameter(const std::string& parameter);
    bool RemoveTriggerParameter(const std::string& parameter);
    std::vector<std::string> GetTriggerParameters() const;
    std::vector<AnimationTransitionInfo> GetTransitions() const;
    bool RemoveTransition(std::size_t index);
    bool SetTransitionExpectedValue(std::size_t index, bool value);
    void SetTrigger(const std::string& trigger);
    void ResetTrigger(const std::string& trigger);

    void Update(float deltaTime) override;
    std::unique_ptr<Component> Clone() const override;

protected:
    void OnStart() override;

private:
    enum class ConditionType { Bool, Trigger, Completed };
    struct Transition
    {
        std::string from;
        std::string to;
        std::string parameter;
        bool expectedValue = true;
        ConditionType condition = ConditionType::Bool;
    };

    std::string defaultState;
    std::string currentState;
    std::unordered_map<std::string, bool> boolParameters;
    std::unordered_set<std::string> triggers;
    std::unordered_set<std::string> triggerParameters;
    std::vector<Transition> transitions;

    bool TransitionTo(const std::string& state);
};
