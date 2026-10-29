#pragma once

#include "../../Engine/Scene/GameObject.h"
#include <vector>

struct TransformState
{
    Vector2 position;
    float rotation;
    Vector2 scale;
};

struct TransformHistoryEntry
{
    GameObject* object;
    TransformState before;
    TransformState after;
};

class EditorHistory
{
public:
    void RecordTransform(
        GameObject* object,
        const TransformState& before,
        const TransformState& after
    );

    bool Undo();
    bool Redo();

    bool CanUndo() const;
    bool CanRedo() const;

    void Clear();

private:
    void ApplyTransform(
        GameObject* object,
        const TransformState& state
    );

    std::vector<TransformHistoryEntry> entries;
    int currentIndex = -1;
};