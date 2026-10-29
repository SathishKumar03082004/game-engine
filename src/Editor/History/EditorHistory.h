#pragma once

#include "../../Engine/Scene/Scene.h"
#include <vector>
#include <string>

struct GameObjectSnapshot
{
    std::string name;
    TransformComponent transform;
    int parentIndex = -1;
};

struct SceneSnapshot
{
    std::vector<GameObjectSnapshot> objects;
};

class EditorHistory
{
public:
    void RecordBeforeAction(Scene& scene);
    void Undo(Scene& scene);
    void Redo(Scene& scene);

    bool CanUndo() const;
    bool CanRedo() const;

private:
    std::vector<SceneSnapshot> undoStack;
    std::vector<SceneSnapshot> redoStack;

    SceneSnapshot CaptureScene(Scene& scene);
    void RestoreScene(Scene& scene, const SceneSnapshot& snapshot);
};