#include "Hierarchy.h"

Hierarchy::Hierarchy(
    Scene& sceneReference,
    EditorHistory& historyReference
)
    : scene(sceneReference),
      history(historyReference),
      panel{0.0f, 0.0f, 250.0f, 720.0f},
      createButton{10.0f, 675.0f, 105.0f, 30.0f},
      deleteButton{120.0f, 675.0f, 105.0f, 30.0f},
      width(250.0f),
      height(720.0f),
      isRenaming(false),
      renamingObject(nullptr)
{
}

void Hierarchy::Initialize()
{
    height = static_cast<float>(GetScreenHeight());

    panel = {
        0.0f,
        0.0f,
        width,
        height
    };
}

void Hierarchy::Update()
{
    height = static_cast<float>(GetScreenHeight());
    panel.height = height;

    createButton = {
        10.0f,
        height - 45.0f,
        105.0f,
        30.0f
    };

    deleteButton = {
        120.0f,
        height - 45.0f,
        105.0f,
        30.0f
    };

    if (isRenaming)
    {
        HandleRenameInput();
        return;
    }

    if (IsKeyPressed(KEY_F2))
    {
        StartRenaming();
        return;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D))
    {
        DuplicateSelectedObject();
        return;
    }

    if (IsKeyPressed(KEY_DELETE))
    {
        DeleteSelectedObject();
        return;
    }

    if (!IsMouseOver())
    {
        return;
    }

    Vector2 mouse = GetMousePosition();

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
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

    bool ctrlPressed =
        IsKeyDown(KEY_LEFT_CONTROL) ||
        IsKeyDown(KEY_RIGHT_CONTROL);

    float y = 45.0f;

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
    if (IsKeyPressed(KEY_ENTER))
    {
        CommitRenaming();
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        CancelRenaming();
        return;
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !renameBuffer.empty())
    {
        renameBuffer.pop_back();
    }

    int key = GetCharPressed();

    while (key > 0)
    {
        if (key >= 32 && key <= 126 && renameBuffer.length() < 32)
        {
            renameBuffer += static_cast<char>(key);
        }

        key = GetCharPressed();
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
                ToggleExpanded(object);
                return true;
            }
        }

        bool ctrlPressed =
            IsKeyDown(KEY_LEFT_CONTROL) ||
            IsKeyDown(KEY_RIGHT_CONTROL);

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

void Hierarchy::Draw()
{
    DrawRectangleRec(panel, LIGHTGRAY);
    DrawRectangleLinesEx(panel, 2.0f, DARKGRAY);

    DrawText("Hierarchy", 10, 10, 20, BLACK);

    float y = 45.0f;

    auto& objects = scene.GetGameObjects();

    for (auto& object : objects)
    {
        if (object->GetParent() == nullptr)
        {
            DrawObject(object.get(), 0, y);
        }
    }

    DrawRectangleRec(createButton, DARKGRAY);

    DrawText(
        "+ Create",
        static_cast<int>(createButton.x + 10.0f),
        static_cast<int>(createButton.y + 7.0f),
        15,
        WHITE
    );

    DrawRectangleRec(deleteButton, DARKGRAY);

    DrawText(
        "- Delete",
        static_cast<int>(deleteButton.x + 10.0f),
        static_cast<int>(deleteButton.y + 7.0f),
        15,
        WHITE
    );
}

bool Hierarchy::IsMouseOver() const
{
    return CheckCollisionPointRec(GetMousePosition(), panel);
}