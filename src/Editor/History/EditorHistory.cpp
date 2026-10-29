#include "EditorHistory.h"

void EditorHistory::RecordTransform(
    GameObject* object,
    const TransformState& before,
    const TransformState& after
)
{
    if (object == nullptr)
    {
        return;
    }

    if (before.position.x == after.position.x &&
        before.position.y == after.position.y &&
        before.rotation == after.rotation &&
        before.scale.x == after.scale.x &&
        before.scale.y == after.scale.y)
    {
        return;
    }

    if (currentIndex + 1 < static_cast<int>(entries.size()))
    {
        entries.erase(
            entries.begin() + currentIndex + 1,
            entries.end()
        );
    }

    entries.push_back({object, before, after});
    currentIndex = static_cast<int>(entries.size()) - 1;
}

bool EditorHistory::Undo()
{
    if (!CanUndo())
    {
        return false;
    }

    TransformHistoryEntry& entry = entries[currentIndex];

    if (entry.object == nullptr)
    {
        return false;
    }

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

    TransformHistoryEntry& entry = entries[currentIndex + 1];

    if (entry.object == nullptr)
    {
        return false;
    }

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