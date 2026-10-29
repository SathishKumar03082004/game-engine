#pragma once

#include "../../Engine/Scene/GameObject.h"
#include <vector>

struct TransformState
{
    Vector2 position{};
    float rotation = 0.0f;
    Vector2 scale{1.0f, 1.0f};
};

struct TransformHistoryEntry
{
    GameObject* object = nullptr;
    TransformState before{};
    TransformState after{};
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