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

    float y = 45.0f;

    auto& objects = scene.GetGameObjects();

    for (auto& object : objects)
    {
        if (object->GetParent() == nullptr)
        {
            Rectangle objectRect =
            {
                5.0f,
                y,
                width - 10.0f,
                30.0f
            };

            if (
                IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                CheckCollisionPointRec(mouse, objectRect)
            )
            {
                SelectObject(object.get());
                return;
            }

            y += 30.0f;

            for (GameObject* child : object->GetChildren())
            {
                Rectangle childRect =
                {
                    25.0f,
                    y,
                    width - 30.0f,
                    30.0f
                };

                if (
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                    CheckCollisionPointRec(mouse, childRect)
                )
                {
                    SelectObject(child);
                    return;
                }

                y += 30.0f;
            }
        }
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
        DrawText(
            "-",
            static_cast<int>(objectRect.x + 5.0f),
            static_cast<int>(objectRect.y + 7.0f),
            18,
            WHITE
        );
    }

    DrawText(
        object->GetName().c_str(),
        static_cast<int>(objectRect.x + 20.0f),
        static_cast<int>(objectRect.y + 7.0f),
        16,
        WHITE
    );

    y += objectHeight;

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

    scene.DestroyGameObject(selected);

    selectedIndex = -1;
}

void Hierarchy::Draw()
{
    DrawRectangleRec(
        panel,
        DARKGRAY
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
        WHITE
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
        BLACK
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
        BLACK
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