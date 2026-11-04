#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

struct EventSubscription
{
    std::type_index type{typeid(void)};
    std::size_t id = 0;
};

class EventSystem
{
public:
    static EventSystem& GetInstance()
    {
        static EventSystem system;
        return system;
    }

    template <typename Event, typename Callback>
    EventSubscription Subscribe(Callback&& callback)
    {
        const std::type_index type(typeid(Event));
        const std::size_t id = ++nextId;
        auto wrapper = [function = std::function<void(const Event&)>(std::forward<Callback>(callback))](const void* event)
            { function(*static_cast<const Event*>(event)); };
        listeners[type].push_back({id, std::move(wrapper)});
        return {type, id};
    }

    void Unsubscribe(EventSubscription subscription)
    {
        auto found = listeners.find(subscription.type);
        if (found == listeners.end()) return;
        auto& slots = found->second;
        slots.erase(std::remove_if(slots.begin(), slots.end(), [subscription](const Slot& slot)
            { return slot.id == subscription.id; }), slots.end());
        if (slots.empty()) listeners.erase(found);
    }

    template <typename Event>
    void Publish(const Event& event) const
    {
        const auto found = listeners.find(std::type_index(typeid(Event)));
        if (found == listeners.end()) return;
        const auto snapshot = found->second;
        for (const Slot& slot : snapshot) slot.callback(&event);
    }

    void Clear() { listeners.clear(); }

private:
    struct Slot
    {
        std::size_t id;
        std::function<void(const void*)> callback;
    };
    std::size_t nextId = 0;
    std::unordered_map<std::type_index, std::vector<Slot>> listeners;
};
