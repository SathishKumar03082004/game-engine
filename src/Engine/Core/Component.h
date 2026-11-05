#pragma once
#include <memory>

class GameObject;

class Component
{
public:
    virtual ~Component() = default;

    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    GameObject* GetOwner() const { return owner; }
    bool IsEnabled() const { return enabled; }

    void SetEnabled(bool value)
    {
        if (enabled == value) return;
        enabled = value;
        if (owner != nullptr)
        {
            if (enabled) OnEnable();
            else OnDisable();
        }
    }

    virtual void Update(float deltaTime) { static_cast<void>(deltaTime); }
    virtual std::unique_ptr<Component> Clone() const { return nullptr; }

protected:
    Component() = default;
    virtual void OnCreate() {}
    virtual void OnStart() {}
    virtual void OnDestroy() {}
    virtual void OnEnable() {}
    virtual void OnDisable() {}

private:
    friend class GameObject;

    void Attach(GameObject* newOwner)
    {
        owner = newOwner;
        OnCreate();
        if (!enabled) OnDisable();
        if (started) OnStart();
    }

    void Start()
    {
        if (!started)
        {
            started = true;
            OnStart();
        }
    }

    void Detach()
    {
        if (owner != nullptr)
        {
            OnDestroy();
            owner = nullptr;
        }
    }

    GameObject* owner = nullptr;
    bool enabled = true;
    bool started = false;
};
