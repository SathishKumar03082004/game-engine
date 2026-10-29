#include "EditorHistory.h"

void EditorHistory::RecordBeforeAction(Scene& scene)
{
    undoStack.push_back(CaptureScene(scene));
    redoStack.clear();

    if (undoStack.size() > 100)
    {
        undoStack.erase(undoStack.begin());
    }
}

void EditorHistory::Undo(Scene& scene)
{
    if (undoStack.empty())
    {
        return;
    }

    redoStack.push_back(CaptureScene(scene));

    SceneSnapshot previousState = undoStack.back();
    undoStack.pop_back();

    RestoreScene(scene, previousState);
}

void EditorHistory::Redo(Scene& scene)
{
    if (redoStack.empty())
    {
        return;
    }

    undoStack.push_back(CaptureScene(scene));

    SceneSnapshot nextState = redoStack.back();
    redoStack.pop_back();

    RestoreScene(scene, nextState);
}

bool EditorHistory::CanUndo() const
{
    return !undoStack.empty();
}

bool EditorHistory::CanRedo() const
{
    return !redoStack.empty();
}

SceneSnapshot EditorHistory::CaptureScene(Scene& scene)
{
    SceneSnapshot snapshot;

    auto& objects = scene.GetGameObjects();

    for (const auto& object : objects)
    {
        if (object == nullptr)
        {
            continue;
        }

        GameObjectSnapshot item;
        item.name = object->GetName();
        item.transform = object->GetTransform();

        if (object->GetParent() != nullptr)
        {
            for (int i = 0; i < static_cast<int>(objects.size()); ++i)
            {
                if (objects[i].get() == object->GetParent())
                {
                    item.parentIndex = i;
                    break;
                }
            }
        }

        snapshot.objects.push_back(item);
    }

    return snapshot;
}

void EditorHistory::RestoreScene(
    Scene& scene,
    const SceneSnapshot& snapshot
)
{
    auto& currentObjects = scene.GetGameObjects();

    scene.ClearSelection();
    currentObjects.clear();

    std::vector<GameObject*> restoredObjects;
    restoredObjects.reserve(snapshot.objects.size());

    for (const GameObjectSnapshot& item : snapshot.objects)
    {
        GameObject* object = scene.CreateGameObject(item.name);

        if (object == nullptr)
        {
            continue;
        }

        object->GetTransform() = item.transform;
        restoredObjects.push_back(object);
    }

    for (int i = 0; i < static_cast<int>(snapshot.objects.size()); ++i)
    {
        int parentIndex = snapshot.objects[i].parentIndex;

        if (i >= static_cast<int>(restoredObjects.size()))
        {
            break;
        }

        if (
            parentIndex >= 0 &&
            parentIndex < static_cast<int>(restoredObjects.size()) &&
            parentIndex != i
        )
        {
            scene.SetParent(
                restoredObjects[i],
                restoredObjects[parentIndex]
            );
        }
    }
}