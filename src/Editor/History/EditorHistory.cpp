#include "EditorHistory.h"
#include <utility>

void EditorHistory::RecordTransform(
    GameObject* object,
    const TransformState& before,
    const TransformState& after
)
{
    RecordTransforms({TransformHistoryEntry{object, before, after}});
}

void EditorHistory::RecordTransforms(const std::vector<TransformHistoryEntry>& transforms)
{
    std::vector<TransformHistoryEntry> changed;
    for (const TransformHistoryEntry& entry : transforms)
    {
        if (entry.object == nullptr) continue;
        const TransformState& before = entry.before;
        const TransformState& after = entry.after;
        if (before.position.x == after.position.x && before.position.y == after.position.y &&
            before.rotation == after.rotation && before.scale.x == after.scale.x &&
            before.scale.y == after.scale.y) continue;
        changed.push_back(entry);
    }
    if (changed.empty()) return;
    if (currentIndex + 1 < static_cast<int>(entries.size()))
    {
        entries.erase(
            entries.begin() + currentIndex + 1,
            entries.end()
        );
    }

    entries.push_back(std::move(changed));
    currentIndex = static_cast<int>(entries.size()) - 1;
}

bool EditorHistory::Undo()
{
    if (!CanUndo())
    {
        return false;
    }

    for (const TransformHistoryEntry& entry : entries[currentIndex])
        ApplyTransform(entry.object, entry.before);
    --currentIndex;

    return true;
}

bool EditorHistory::Redo()
{
    if (!CanRedo())
    {
        return false;
    }

    for (const TransformHistoryEntry& entry : entries[currentIndex + 1])
        ApplyTransform(entry.object, entry.after);
    ++currentIndex;

    return true;
}

bool EditorHistory::CanUndo() const
{
    return currentIndex >= 0;
}

bool EditorHistory::CanRedo() const
{
    return currentIndex + 1 < static_cast<int>(entries.size());
}

void EditorHistory::Clear()
{
    entries.clear();
    currentIndex = -1;
}

void EditorHistory::ApplyTransform(
    GameObject* object,
    const TransformState& state
)
{
    if (object == nullptr)
    {
        return;
    }

    TransformComponent& transform = object->GetTransform();

    transform.SetPosition(state.position);
    transform.SetRotation(state.rotation);
    transform.SetScale(state.scale);
}
