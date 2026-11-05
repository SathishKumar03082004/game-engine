#include "../../Engine/Core/InputManager.h"
#include "Hierarchy.h"
#include "../../Engine/Scene/PrefabSystem.h"
#include "../../Engine/UI/UIComponent.h"
#include "rlgl.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace
{
    Vector2 GetHierarchyMousePosition()
    {
        Vector2 mouse = InputManager::GetInstance().GetMousePosition();
        mouse.y -= 27.0f;
        return mouse;
    }
}

Hierarchy::Hierarchy(
    Scene& sceneReference,
    EditorHistory& historyReference
)
    : scene(sceneReference),
      history(historyReference),
      panel{0.0f, 0.0f, 250.0f, 720.0f},
      createButton{5.0f, 675.0f, 56.0f, 30.0f},
      deleteButton{65.0f, 675.0f, 56.0f, 30.0f},
      saveButton{125.0f, 675.0f, 56.0f, 30.0f},
      loadButton{185.0f, 675.0f, 56.0f, 30.0f},
      savePrefabButton{5.0f, 638.0f, 110.0f, 28.0f},
      instantiatePrefabButton{120.0f, 638.0f, 121.0f, 28.0f},
      width(250.0f),
      height(720.0f),
      isRenaming(false),
      renamingObject(nullptr),
      dragCandidate(nullptr),
      dragStartPosition{},
      isDraggingObject(false),
      loadConfirmationPending(false)
{
}

void Hierarchy::Initialize()
{
    height = static_cast<float>(GetScreenHeight()) - 27.0f;

    panel = {
        0.0f,
        0.0f,
        width,
        height
    };
}

void Hierarchy::Update()
{
    if (!visible) return;
    height = static_cast<float>(GetScreenHeight()) - 27.0f;
    panel.height = height;

    createButton = {
        5.0f,
        height - 45.0f,
        56.0f,
        30.0f
    };

    deleteButton = {
        65.0f,
        height - 45.0f,
        56.0f,
        30.0f
    };

    saveButton = {
        125.0f,
        height - 45.0f,
        56.0f,
        30.0f
    };

    loadButton = {
        185.0f,
        height - 45.0f,
        56.0f,
        30.0f
    };
    savePrefabButton = {5.0f, height - 82.0f, 110.0f, 28.0f};
    instantiatePrefabButton = {120.0f, height - 82.0f, 121.0f, 28.0f};

    if (loadConfirmationPending && InputManager::GetInstance().IsKeyPressed(KEY_ESCAPE))
    {
        loadConfirmationPending = false;
        saveStatus = "Load cancelled.";
    }

    if (dragCandidate != nullptr && InputManager::GetInstance().IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        const Vector2 mouse = GetHierarchyMousePosition();
        const float dx = mouse.x - dragStartPosition.x;
        const float dy = mouse.y - dragStartPosition.y;
        if (dx * dx + dy * dy >= 25.0f)
        {
            isDraggingObject = true;
        }
    }

    if (InputManager::GetInstance().IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && dragCandidate != nullptr)
    {
        if (isDraggingObject)
        {
            FinishObjectDrag(GetHierarchyMousePosition());
        }
        dragCandidate = nullptr;
        isDraggingObject = false;
    }

    if (isRenaming)
    {
        HandleRenameInput();
        return;
    }

    if (InputManager::GetInstance().IsKeyPressed(KEY_F2))
    {
        StartRenaming();
        return;
    }

    if (InputManager::GetInstance().IsKeyDown(KEY_LEFT_CONTROL) && InputManager::GetInstance().IsKeyPressed(KEY_D))
    {
        DuplicateSelectedObject();
        return;
    }

    if (InputManager::GetInstance().IsKeyPressed(KEY_DELETE))
    {
        DeleteSelectedObject();
        return;
    }

    if (!IsMouseOver())
    {
        return;
    }

    Vector2 mouse = GetHierarchyMousePosition();

    if (!InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    if (CheckCollisionPointRec(mouse, createButton))
    {
        CreateObject();
        return;
    }

    if (CheckCollisionPointRec(mouse, deleteButton))
    {
        DeleteSelectedObject();
        return;
    }

    if (CheckCollisionPointRec(mouse, saveButton))
    {
        SaveScene();
        return;
    }

    if (CheckCollisionPointRec(mouse, loadButton))
    {
        LoadScene();
        return;
    }

    if (CheckCollisionPointRec(mouse, savePrefabButton))
    {
        SaveSelectedPrefab();
        return;
    }
    if (CheckCollisionPointRec(mouse, instantiatePrefabButton))
    {
        InstantiateMostRecentPrefab();
        return;
    }

    bool ctrlPressed =
        InputManager::GetInstance().IsKeyDown(KEY_LEFT_CONTROL) ||
        InputManager::GetInstance().IsKeyDown(KEY_RIGHT_CONTROL);

    float y = 52.0f;

    auto& objects = scene.GetGameObjects();

    for (auto& object : objects)
    {
        if (object->GetParent() == nullptr)
        {
            if (HandleObjectClick(object.get(), 0, y, mouse))
            {
                return;
            }
        }
    }

    if (!ctrlPressed)
    {
        scene.ClearSelection();
    }
}

void Hierarchy::HandleRenameInput()
{
    if (InputManager::GetInstance().IsKeyPressed(KEY_ENTER))
    {
        CommitRenaming();
        return;
    }

    if (InputManager::GetInstance().IsKeyPressed(KEY_ESCAPE))
    {
        CancelRenaming();
        return;
    }

    if (InputManager::GetInstance().IsKeyPressed(KEY_BACKSPACE) && !renameBuffer.empty())
    {
        renameBuffer.pop_back();
    }

    int key = InputManager::GetInstance().GetCharPressed();

    while (key > 0)
    {
        if (key >= 32 && key <= 126 && renameBuffer.length() < 32)
        {
            renameBuffer += static_cast<char>(key);
        }

        key = InputManager::GetInstance().GetCharPressed();
    }
}

void Hierarchy::StartRenaming()
{
    GameObject* selected = scene.GetSelectedObject();

    if (selected == nullptr)
    {
        return;
    }

    isRenaming = true;
    renamingObject = selected;
    renameBuffer = selected->GetName();
}

void Hierarchy::CommitRenaming()
{
    if (renamingObject == nullptr || renameBuffer.empty())
    {
        CancelRenaming();
        return;
    }

    renamingObject->SetName(renameBuffer);

    isRenaming = false;
    renamingObject = nullptr;
    renameBuffer.clear();
}

void Hierarchy::CancelRenaming()
{
    isRenaming = false;
    renamingObject = nullptr;
    renameBuffer.clear();
}

bool Hierarchy::HandleObjectClick(
    GameObject* object,
    int depth,
    float& y,
    Vector2 mouse
)
{
    if (object == nullptr)
    {
        return false;
    }

    const float objectHeight = 30.0f;
    const float indentation = depth * 20.0f;

    Rectangle objectRect = {
        5.0f + indentation,
        y,
        width - 10.0f - indentation,
        objectHeight
    };

    if (CheckCollisionPointRec(mouse, objectRect))
    {
        if (object->HasChildren())
        {
            Rectangle arrowRect = {
                objectRect.x,
                objectRect.y,
                20.0f,
                objectHeight
            };

            if (CheckCollisionPointRec(mouse, arrowRect))
            {
                dragCandidate = nullptr;
                ToggleExpanded(object);
                return true;
            }
        }

        dragCandidate = object;
        dragStartPosition = mouse;
        isDraggingObject = false;

        bool ctrlPressed =
            InputManager::GetInstance().IsKeyDown(KEY_LEFT_CONTROL) ||
            InputManager::GetInstance().IsKeyDown(KEY_RIGHT_CONTROL);

        scene.SelectObject(object, ctrlPressed);
        return true;
    }

    y += objectHeight;

    if (!IsExpanded(object))
    {
        return false;
    }

    for (GameObject* child : object->GetChildren())
    {
        if (HandleObjectClick(child, depth + 1, y, mouse))
        {
            return true;
        }
    }

    return false;
}

GameObject* Hierarchy::FindObjectAt(Vector2 mouse) const
{
    float y = 52.0f;
    const auto& objects = scene.GetGameObjects();

    for (const auto& object : objects)
    {
        if (object != nullptr && object->GetParent() == nullptr)
        {
            if (GameObject* hit = FindObjectAt(object.get(), 0, y, mouse))
            {
                return hit;
            }
        }
    }

    return nullptr;
}

GameObject* Hierarchy::FindObjectAt(
    GameObject* object,
    int depth,
    float& y,
    Vector2 mouse
) const
{
    if (object == nullptr)
    {
        return nullptr;
    }

    const float indentation = depth * 20.0f;
    const Rectangle row = {
        5.0f + indentation,
        y,
        width - 10.0f - indentation,
        30.0f
    };

    if (CheckCollisionPointRec(mouse, row))
    {
        return object;
    }

    y += 30.0f;
    if (!IsExpanded(object))
    {
        return nullptr;
    }

    for (GameObject* child : object->GetChildren())
    {
        if (GameObject* hit = FindObjectAt(child, depth + 1, y, mouse))
        {
            return hit;
        }
    }

    return nullptr;
}

void Hierarchy::FinishObjectDrag(Vector2 mouse)
{
    if (dragCandidate == nullptr || !IsMouseOver())
    {
        return;
    }

    const GameObject* candidate = dragCandidate;
    GameObject* target = FindObjectAt(mouse);

    if (target != nullptr)
    {
        if (target != candidate)
        {
            history.Clear();
            scene.SetParent(dragCandidate, target);
        }
        return;
    }

    // Dropping in the blank hierarchy area makes the object a root object.
    if (mouse.y >= 52.0f && mouse.y < deleteButton.y &&
        dragCandidate->GetParent() != nullptr)
    {
        history.Clear();
        scene.ClearParent(dragCandidate);
    }
}

void Hierarchy::DrawObject(
    GameObject* object,
    int depth,
    float& y
)
{
    if (object == nullptr)
    {
        return;
    }

    const float objectHeight = 30.0f;
    const float indentation = depth * 20.0f;

    Rectangle objectRect = {
        5.0f + indentation,
        y,
        width - 10.0f - indentation,
        objectHeight
    };

    if (scene.IsObjectSelected(object))
    {
        DrawRectangleRec(objectRect, SKYBLUE);
    }

    if (object->HasChildren())
    {
        DrawText(
            IsExpanded(object) ? "v" : ">",
            static_cast<int>(objectRect.x + 5.0f),
            static_cast<int>(objectRect.y + 7.0f),
            16,
            BLACK
        );
    }

    if (isRenaming && renamingObject == object)
    {
        Rectangle nameField = {
            objectRect.x + 20.0f,
            objectRect.y + 4.0f,
            objectRect.width - 25.0f,
            22.0f
        };

        DrawRectangleRec(nameField, WHITE);
        DrawRectangleLinesEx(nameField, 1.0f, DARKGRAY);

        DrawText(
            renameBuffer.c_str(),
            static_cast<int>(nameField.x + 4.0f),
            static_cast<int>(nameField.y + 3.0f),
            16,
            BLACK
        );
    }
    else
    {
        DrawText(
            object->GetName().c_str(),
            static_cast<int>(objectRect.x + 22.0f),
            static_cast<int>(objectRect.y + 7.0f),
            16,
            BLACK
        );
    }

    y += objectHeight;

    if (!IsExpanded(object))
    {
        return;
    }

    for (GameObject* child : object->GetChildren())
    {
        DrawObject(child, depth + 1, y);
    }
}

bool Hierarchy::IsExpanded(GameObject* object) const
{
    if (object == nullptr)
    {
        return false;
    }

    return collapsedObjects.find(object) == collapsedObjects.end();
}

void Hierarchy::ToggleExpanded(GameObject* object)
{
    if (object == nullptr)
    {
        return;
    }

    auto it = collapsedObjects.find(object);

    if (it == collapsedObjects.end())
    {
        collapsedObjects.insert(object);
    }
    else
    {
        collapsedObjects.erase(it);
    }
}

void Hierarchy::CreateObject()
{
    // Clear transform history before changing scene objects.
    history.Clear();

    GameObject* object = scene.CreateGameObject();

    if (object != nullptr)
    {
        scene.SelectObject(object);
    }
}

void Hierarchy::DeleteSelectedObject()
{
    if (isRenaming)
    {
        return;
    }

    const auto selectedObjects = scene.GetSelectedObjects();

    if (selectedObjects.empty())
    {
        return;
    }

    history.Clear();

    std::vector<GameObject*> objectsToDelete(
        selectedObjects.begin(),
        selectedObjects.end()
    );

    for (GameObject* object : objectsToDelete)
    {
        if (object == nullptr)
        {
            continue;
        }

        collapsedObjects.erase(object);
        scene.DestroyGameObject(object);
    }
}

void Hierarchy::DuplicateSelectedObject()
{
    if (isRenaming)
    {
        return;
    }

    GameObject* selected = scene.GetSelectedObject();

    if (selected == nullptr)
    {
        return;
    }

    history.Clear();

    GameObject* duplicate = scene.DuplicateGameObject(selected);

    if (duplicate != nullptr)
    {
        scene.SelectObject(duplicate);
    }
}

void Hierarchy::SaveScene()
{
    saveStatus = scene.SaveToFile("resources/scenes/current.scene")
        ? "Saved: resources/scenes/current.scene"
        : "Save failed. Check file permissions.";
}

void Hierarchy::LoadScene()
{
    if (!loadConfirmationPending)
    {
        loadConfirmationPending = true;
        saveStatus = "Click Load again to confirm; Esc cancels.";
        return;
    }

    loadConfirmationPending = false;
    if (scene.LoadFromFile("resources/scenes/current.scene"))
    {
        history.Clear();
        collapsedObjects.clear();
        dragCandidate = nullptr;
        isDraggingObject = false;
        saveStatus = "Loaded: resources/scenes/current.scene";
    }
    else
    {
        saveStatus = "Load failed. Scene file or referenced assets are invalid.";
    }
}

void Hierarchy::SaveSelectedPrefab()
{
    GameObject* selected = scene.GetSelectedObject();
    if (selected == nullptr)
    {
        saveStatus = "Select an object to save as a prefab.";
        return;
    }

    std::string fileName = selected->GetName();
    for (char& character : fileName)
    {
        const unsigned char value = static_cast<unsigned char>(character);
        if (!std::isalnum(value) && character != '-' && character != '_') character = '_';
    }
    if (fileName.empty()) fileName = "GameObject";
    lastPrefabPath = (std::filesystem::path("resources/prefabs") / (fileName + ".prefab")).string();
    saveStatus = PrefabSystem::Save(*selected, lastPrefabPath)
        ? "Saved prefab: " + lastPrefabPath
        : "Prefab save failed.";
}

void Hierarchy::InstantiateMostRecentPrefab()
{
    std::error_code error;
    const std::filesystem::path directory("resources/prefabs");
    if (lastPrefabPath.empty())
    {
        std::filesystem::file_time_type newest{};
        for (std::filesystem::directory_iterator it(directory, error), end; !error && it != end; it.increment(error))
        {
            if (it->path().extension() != ".prefab" || !it->is_regular_file(error) || error) continue;
            const auto writeTime = it->last_write_time(error);
            if (error) { error.clear(); continue; }
            if (lastPrefabPath.empty() || writeTime > newest)
            {
                newest = writeTime;
                lastPrefabPath = it->path().string();
            }
        }
    }
    if (lastPrefabPath.empty())
    {
        saveStatus = "No prefab found. Save a selected object first.";
        return;
    }

    GameObject* created = PrefabSystem::Instantiate(scene, lastPrefabPath);
    if (created != nullptr) history.Clear();
    saveStatus = created == nullptr ? "Prefab instantiate failed: " + lastPrefabPath
                                    : "Instantiated prefab: " + lastPrefabPath;
}

void Hierarchy::Draw()
{
    if (!visible) return;
    rlPushMatrix();
    rlTranslatef(0.0f, 27.0f, 0.0f);
    DrawRectangleRec(panel, LIGHTGRAY);
    DrawRectangleLinesEx(panel, 2.0f, DARKGRAY);

    DrawText("Hierarchy", 10, 10, 20, BLACK);
    DrawText("Drag onto a row to parent; empty space to unparent", 10, 32, 10, DARKGRAY);

    float y = 52.0f;

    auto& objects = scene.GetGameObjects();

    for (auto& object : objects)
    {
        if (object->GetParent() == nullptr)
        {
            DrawObject(object.get(), 0, y);
        }
    }

    const bool operationSucceeded =
        saveStatus.rfind("Saved:", 0) == 0 ||
        saveStatus.rfind("Loaded:", 0) == 0 ||
        saveStatus.rfind("Saved prefab:", 0) == 0 ||
        saveStatus.rfind("Instantiated prefab:", 0) == 0;
    DrawText(saveStatus.c_str(), 8, static_cast<int>(height - 104.0f), 10,
        operationSucceeded ? DARKGREEN : MAROON);

    DrawRectangleRec(createButton, DARKGRAY);

    DrawText(
        "+ New",
        static_cast<int>(createButton.x + 6.0f),
        static_cast<int>(createButton.y + 7.0f),
        12,
        WHITE
    );

    DrawRectangleRec(deleteButton, DARKGRAY);

    DrawText(
        "Delete",
        static_cast<int>(deleteButton.x + 6.0f),
        static_cast<int>(deleteButton.y + 7.0f),
        12,
        WHITE
    );

    DrawRectangleRec(saveButton, DARKGRAY);
    DrawText("Save", static_cast<int>(saveButton.x + 12.0f),
        static_cast<int>(saveButton.y + 7.0f), 12, WHITE);
    DrawRectangleRec(loadButton, loadConfirmationPending ? ORANGE : DARKGRAY);
    DrawText(loadConfirmationPending ? "Confirm" : "Load",
        static_cast<int>(loadButton.x + 5.0f),
        static_cast<int>(loadButton.y + 7.0f), 12, WHITE);
    DrawRectangleRec(savePrefabButton, DARKGRAY);
    DrawText("Save Prefab", static_cast<int>(savePrefabButton.x + 12.0f),
        static_cast<int>(savePrefabButton.y + 7.0f), 12, WHITE);
    DrawRectangleRec(instantiatePrefabButton, DARKGRAY);
    DrawText("Use Prefab", static_cast<int>(instantiatePrefabButton.x + 19.0f),
        static_cast<int>(instantiatePrefabButton.y + 7.0f), 12, WHITE);
    rlPopMatrix();
}

bool Hierarchy::IsMouseOver() const
{
    if (!visible) return false;
    Vector2 mouse = InputManager::GetInstance().GetMousePosition();
    if (mouse.y < 27.0f) return false;
    mouse.y -= 27.0f;
    return CheckCollisionPointRec(mouse, panel);
}
