#include "Hierarchy.h"

Hierarchy::Hierarchy(Scene& sceneReference)
    : scene(sceneReference)
{
    width = 250.0f;
    height = 720.0f;
    selectedIndex = -1;
}

void Hierarchy::Initialize()
{
    height = static_cast<float>(GetScreenHeight());

    panel =
    {
        0,
        0,
        width,
        height
    };
}

void Hierarchy::Update()
{
    height = static_cast<float>(GetScreenHeight());

    panel.height = height;

    createButton =
    {
        10.0f,
        height - 45.0f,
        105.0f,
        30.0f
    };

    deleteButton =
    {
        120.0f,
        height - 45.0f,
        105.0f,
        30.0f
    };

    if (IsKeyPressed(KEY_DELETE))
    {
        DeleteSelectedObject();
    }

    if (!IsMouseOver())
    {
        return;
    }

    Vector2 mouse = GetMousePosition();

    if (
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mouse, createButton)
    )
    {
        CreateObject();
        return;
    }

    if (
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mouse, deleteButton)
    )
    {
        DeleteSelectedObject();
        return;
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    float y = 45.0f;

    auto& objects = scene.GetGameObjects();

    for (auto& object : objects)
    {
        if (object->GetParent() == nullptr)
        {
            if (
                HandleObjectClick(
                    object.get(),
                    0,
                    y,
                    mouse
                )
            )
            {
                return;
            }
        }
    }
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

    Rectangle objectRect =
    {
        5.0f + indentation,
        y,
        width - 10.0f - indentation,
        objectHeight
    };

    if (CheckCollisionPointRec(mouse, objectRect))
    {
        if (object->HasChildren())
        {
            Rectangle arrowRect =
            {
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

        SelectObject(object);
        return true;
    }

    y += objectHeight;

    if (!IsExpanded(object))
    {
        return false;
    }

    for (GameObject* child : object->GetChildren())
    {
        if (
            HandleObjectClick(
                child,
                depth + 1,
                y,
                mouse
            )
        )
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

    float indentation = depth * 20.0f;

    Rectangle objectRect =
    {
        5.0f + indentation,
        y,
        width - 10.0f - indentation,
        objectHeight
    };

    if (object == scene.GetSelectedObject())
    {
        DrawRectangleRec(
            objectRect,
            SKYBLUE
        );
    }

    if (object->HasChildren())
    {
        if (IsExpanded(object))
        {
            DrawText(
                "v",
                static_cast<int>(objectRect.x + 4.0f),
                static_cast<int>(objectRect.y + 7.0f),
                16,
                BLACK
            );
        }
        else
        {
            DrawText(
                ">",
                static_cast<int>(objectRect.x + 5.0f),
                static_cast<int>(objectRect.y + 7.0f),
                16,
                BLACK
            );
        }
    }

    DrawText(
        object->GetName().c_str(),
        static_cast<int>(objectRect.x + 22.0f),
        static_cast<int>(objectRect.y + 7.0f),
        16,
        BLACK
    );

    y += objectHeight;

    if (!IsExpanded(object))
    {
        return;
    }

    for (GameObject* child : object->GetChildren())
    {
        DrawObject(
            child,
            depth + 1,
            y
        );
    }
}

void Hierarchy::SelectObject(
    GameObject* object
)
{
    if (object == nullptr)
    {
        return;
    }

    scene.SelectObject(object);
}

bool Hierarchy::IsExpanded(
    GameObject* object
) const
{
    if (object == nullptr)
    {
        return false;
    }

    return collapsedObjects.find(object)
        == collapsedObjects.end();
}

void Hierarchy::ToggleExpanded(
    GameObject* object
)
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
    GameObject* object =
        scene.CreateGameObject();

    if (object == nullptr)
    {
        return;
    }

    scene.SelectObject(object);
}

void Hierarchy::DeleteSelectedObject()
{
    GameObject* selected =
        scene.GetSelectedObject();

    if (selected == nullptr)
    {
        return;
    }

    collapsedObjects.erase(selected);

    scene.DestroyGameObject(selected);

    selectedIndex = -1;
}

void Hierarchy::Draw()
{
    DrawRectangleRec(
        panel,
        LIGHTGRAY
    );

    DrawRectangleLinesEx(
        panel,
        2.0f,
        DARKGRAY
    );

    DrawText(
        "Hierarchy",
        10,
        10,
        20,
        BLACK
    );

    float y = 45.0f;

    auto& objects = scene.GetGameObjects();

    for (auto& object : objects)
    {
        if (object->GetParent() == nullptr)
        {
            DrawObject(
                object.get(),
                0,
                y
            );
        }
    }

    DrawRectangleRec(
        createButton,
        DARKGRAY
    );

    DrawText(
        "+ Create",
        static_cast<int>(createButton.x + 10.0f),
        static_cast<int>(createButton.y + 7.0f),
        15,
        WHITE
    );

    DrawRectangleRec(
        deleteButton,
        DARKGRAY
    );

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
    return CheckCollisionPointRec(
        GetMousePosition(),
        panel
    );
}