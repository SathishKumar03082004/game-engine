
#include "Inspector.h"

#include <cstdlib>
#include <string>

// --------------------------------------------------
// Constructor
// --------------------------------------------------

Inspector::Inspector(Scene& sceneReference)
    : scene(sceneReference)
{
    width = 300.0f;
    height = static_cast<float>(GetScreenHeight());

    panel =
    {
        static_cast<float>(GetScreenWidth()) - width,
        0.0f,
        width,
        height
    };

    activeField = ActiveField::None;
    inputBuffer.clear();
}

// --------------------------------------------------
// Initialize
// --------------------------------------------------

void Inspector::Initialize()
{
}

// --------------------------------------------------
// Update
// --------------------------------------------------

void Inspector::Update()
{
    width = 300.0f;
    height = static_cast<float>(GetScreenHeight());

    panel.x = static_cast<float>(GetScreenWidth()) - width;
    panel.y = 0.0f;
    panel.width = width;
    panel.height = height;

    Vector2 mousePosition = GetMousePosition();

    GameObject* selectedObject = scene.GetSelectedObject();

    // Continue keyboard editing even if the mouse moves
    // outside the Inspector panel.
    if (activeField != ActiveField::None)
    {
        AddCharacterToInput();

        if (IsKeyPressed(KEY_BACKSPACE))
        {
            if (!inputBuffer.empty())
            {
                inputBuffer.pop_back();
            }
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            CommitEditing();
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
            activeField = ActiveField::None;
            inputBuffer.clear();
        }
    }

    if (selectedObject == nullptr)
    {
        return;
    }

    if (!CheckCollisionPointRec(mousePosition, panel))
    {
        return;
    }

    // --------------------------------------------------
    // Mouse interaction
    // --------------------------------------------------

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    TransformComponent& transform =
        selectedObject->GetTransform();

    Vector2 position = transform.GetPosition();
    Vector2 scale = transform.GetScale();

    float rotation = transform.GetRotation();

    // Position X
    Rectangle positionXField =
    {
        panel.x + 80.0f,
        170.0f,
        90.0f,
        28.0f
    };

    if (CheckCollisionPointRec(mousePosition, positionXField))
    {
        CommitEditing();

        StartEditing(
            ActiveField::PositionX,
            position.x
        );

        return;
    }

    // Position Y
    Rectangle positionYField =
    {
        panel.x + 190.0f,
        170.0f,
        90.0f,
        28.0f
    };

    if (CheckCollisionPointRec(mousePosition, positionYField))
    {
        CommitEditing();

        StartEditing(
            ActiveField::PositionY,
            position.y
        );

        return;
    }

    // Rotation
    Rectangle rotationField =
    {
        panel.x + 80.0f,
        240.0f,
        200.0f,
        28.0f
    };

    if (CheckCollisionPointRec(mousePosition, rotationField))
    {
        CommitEditing();

        StartEditing(
            ActiveField::Rotation,
            rotation
        );

        return;
    }

    // Scale X
    Rectangle scaleXField =
    {
        panel.x + 80.0f,
        310.0f,
        90.0f,
        28.0f
    };

    if (CheckCollisionPointRec(mousePosition, scaleXField))
    {
        CommitEditing();

        StartEditing(
            ActiveField::ScaleX,
            scale.x
        );

        return;
    }

    // Scale Y
    Rectangle scaleYField =
    {
        panel.x + 190.0f,
        310.0f,
        90.0f,
        28.0f
    };

    if (CheckCollisionPointRec(mousePosition, scaleYField))
    {
        CommitEditing();

        StartEditing(
            ActiveField::ScaleY,
            scale.y
        );

        return;
    }

    // Sprite path
    Rectangle spritePathField =
    {
        panel.x + 15.0f,
        435.0f,
        270.0f,
        30.0f
    };

    if (CheckCollisionPointRec(mousePosition, spritePathField))
    {
        CommitEditing();

        activeField = ActiveField::SpritePath;
        inputBuffer.clear();

        return;
    }

    // --------------------------------------------------
    // Sprite tint presets
    // --------------------------------------------------

    SpriteRenderer& sprite =
        selectedObject->GetSpriteRenderer();

    if (mousePosition.y >= 555.0f &&
        mousePosition.y <= 580.0f)
    {
        if (mousePosition.x >= panel.x + 15.0f &&
            mousePosition.x <= panel.x + 50.0f)
        {
            sprite.SetTint(WHITE);
            return;
        }

        if (mousePosition.x >= panel.x + 60.0f &&
            mousePosition.x <= panel.x + 95.0f)
        {
            sprite.SetTint(RED);
            return;
        }

        if (mousePosition.x >= panel.x + 105.0f &&
            mousePosition.x <= panel.x + 140.0f)
        {
            sprite.SetTint(GREEN);
            return;
        }

        if (mousePosition.x >= panel.x + 150.0f &&
            mousePosition.x <= panel.x + 185.0f)
        {
            sprite.SetTint(BLUE);
            return;
        }

        if (mousePosition.x >= panel.x + 195.0f &&
            mousePosition.x <= panel.x + 230.0f)
        {
            sprite.SetTint(YELLOW);
            return;
        }
    }

    // --------------------------------------------------
    // Remove Sprite
    // --------------------------------------------------

    Rectangle removeSpriteButton =
    {
        panel.x + 15.0f,
        615.0f,
        120.0f,
        30.0f
    };

    if (CheckCollisionPointRec(mousePosition, removeSpriteButton))
    {
        selectedObject->ClearSprite();
        return;
    }

    // Clicked elsewhere: commit current editing.
    CommitEditing();
}

// --------------------------------------------------
// Draw
// --------------------------------------------------

void Inspector::Draw()
{
    DrawRectangleRec(panel, DARKGRAY);

    DrawText(
        "INSPECTOR",
        static_cast<int>(panel.x + 15.0f),
        15,
        20,
        WHITE
    );

    GameObject* selectedObject =
        scene.GetSelectedObject();

    if (selectedObject == nullptr)
    {
        DrawText(
            "Nothing Selected",
            static_cast<int>(panel.x + 15.0f),
            60,
            18,
            LIGHTGRAY
        );

        return;
    }

    DrawText(
        selectedObject->GetName().c_str(),
        static_cast<int>(panel.x + 15.0f),
        55,
        22,
        WHITE
    );

    DrawTransform(selectedObject);

    DrawSprite(selectedObject);
}

// --------------------------------------------------
// Transform Inspector
// --------------------------------------------------

void Inspector::DrawTransform(GameObject* object)
{
    TransformComponent& transform =
        object->GetTransform();

    Vector2 position = transform.GetPosition();

    float rotation = transform.GetRotation();

    Vector2 scale = transform.GetScale();

    DrawRectangle(
        static_cast<int>(panel.x + 10.0f),
        95,
        static_cast<int>(width - 20.0f),
        30,
        GRAY
    );

    DrawText(
        "Transform",
        static_cast<int>(panel.x + 20.0f),
        101,
        18,
        WHITE
    );

    DrawText(
        "Position",
        static_cast<int>(panel.x + 20.0f),
        145,
        18,
        WHITE
    );

    DrawField(
        "X",
        TextFormat("%.2f", position.x),
        {
            panel.x + 80.0f,
            170.0f,
            90.0f,
            28.0f
        },
        ActiveField::PositionX
    );

    DrawField(
        "Y",
        TextFormat("%.2f", position.y),
        {
            panel.x + 190.0f,
            170.0f,
            90.0f,
            28.0f
        },
        ActiveField::PositionY
    );

    DrawText(
        "Rotation",
        static_cast<int>(panel.x + 20.0f),
        215,
        18,
        WHITE
    );

    DrawField(
        "",
        TextFormat("%.2f", rotation),
        {
            panel.x + 80.0f,
            240.0f,
            200.0f,
            28.0f
        },
        ActiveField::Rotation
    );

    DrawText(
        "Scale",
        static_cast<int>(panel.x + 20.0f),
        285,
        18,
        WHITE
    );

    DrawField(
        "X",
        TextFormat("%.2f", scale.x),
        {
            panel.x + 80.0f,
            310.0f,
            90.0f,
            28.0f
        },
        ActiveField::ScaleX
    );

    DrawField(
        "Y",
        TextFormat("%.2f", scale.y),
        {
            panel.x + 190.0f,
            310.0f,
            90.0f,
            28.0f
        },
        ActiveField::ScaleY
    );
}

// --------------------------------------------------
// Sprite Renderer Inspector
// --------------------------------------------------

void Inspector::DrawSprite(GameObject* object)
{
    if (object == nullptr)
    {
        return;
    }

    SpriteRenderer& sprite =
        object->GetSpriteRenderer();

    DrawRectangle(
        static_cast<int>(panel.x + 10.0f),
        370,
        static_cast<int>(width - 20.0f),
        30,
        GRAY
    );

    DrawText(
        "Sprite Renderer",
        static_cast<int>(panel.x + 20.0f),
        376,
        18,
        WHITE
    );

    DrawText(
        "Texture:",
        static_cast<int>(panel.x + 15.0f),
        410,
        16,
        WHITE
    );

    Rectangle pathField =
    {
        panel.x + 15.0f,
        435.0f,
        270.0f,
        30.0f
    };

    DrawRectangleRec(
        pathField,
        activeField == ActiveField::SpritePath
            ? LIGHTGRAY
            : BLACK
    );

    const char* pathText =
        sprite.IsLoaded()
            ? sprite.GetFilePath().c_str()
            : "No texture loaded";

    if (activeField == ActiveField::SpritePath)
    {
        pathText = inputBuffer.c_str();
    }

    DrawText(
        pathText,
        static_cast<int>(pathField.x + 5.0f),
        static_cast<int>(pathField.y + 7.0f),
        12,
        activeField == ActiveField::SpritePath
            ? BLACK
            : WHITE
    );

    DrawText(
        "Click field, type path, press Enter",
        static_cast<int>(panel.x + 15.0f),
        470,
        11,
        LIGHTGRAY
    );

    DrawText(
        "Tint:",
        static_cast<int>(panel.x + 15.0f),
        500,
        16,
        WHITE
    );

    Color tint = sprite.GetTint();

    DrawText(
        TextFormat(
            "R:%d  G:%d  B:%d  A:%d",
            tint.r,
            tint.g,
            tint.b,
            tint.a
        ),
        static_cast<int>(panel.x + 15.0f),
        525,
        14,
        WHITE
    );

    // Tint presets
    DrawRectangle(
        static_cast<int>(panel.x + 15.0f),
        555,
        35,
        25,
        WHITE
    );

    DrawRectangle(
        static_cast<int>(panel.x + 60.0f),
        555,
        35,
        25,
        RED
    );

    DrawRectangle(
        static_cast<int>(panel.x + 105.0f),
        555,
        35,
        25,
        GREEN
    );

    DrawRectangle(
        static_cast<int>(panel.x + 150.0f),
        555,
        35,
        25,
        BLUE
    );

    DrawRectangle(
        static_cast<int>(panel.x + 195.0f),
        555,
        35,
        25,
        YELLOW
    );

    DrawText(
        "Tint presets",
        static_cast<int>(panel.x + 15.0f),
        585,
        12,
        LIGHTGRAY
    );

    // Remove Sprite button
    DrawRectangle(
        static_cast<int>(panel.x + 15.0f),
        615,
        120,
        30,
        DARKGRAY
    );

    DrawText(
        "Remove Sprite",
        static_cast<int>(panel.x + 22.0f),
        623,
        14,
        WHITE
    );
}

// --------------------------------------------------
// Draw Field
// --------------------------------------------------

void Inspector::DrawField(
    const char* label,
    const char* value,
    Rectangle fieldRect,
    ActiveField field
)
{
    if (label[0] != '\0')
    {
        DrawText(
            label,
            static_cast<int>(fieldRect.x - 25.0f),
            static_cast<int>(fieldRect.y + 5.0f),
            16,
            WHITE
        );
    }

    bool active = activeField == field;

    Color backgroundColor =
        active ? LIGHTGRAY : GRAY;

    DrawRectangleRec(
        fieldRect,
        backgroundColor
    );

    const char* displayValue = value;

    if (active)
    {
        displayValue = inputBuffer.c_str();
    }

    DrawText(
        displayValue,
        static_cast<int>(fieldRect.x + 8.0f),
        static_cast<int>(fieldRect.y + 5.0f),
        16,
        active ? BLACK : WHITE
    );
}

// --------------------------------------------------
// Start Editing
// --------------------------------------------------

void Inspector::StartEditing(
    ActiveField field,
    float currentValue
)
{
    activeField = field;

    inputBuffer = TextFormat(
        "%.2f",
        currentValue
    );
}

// --------------------------------------------------
// Commit Editing
// --------------------------------------------------

void Inspector::CommitEditing()
{
    if (activeField == ActiveField::None)
    {
        return;
    }

    GameObject* selectedObject =
        scene.GetSelectedObject();

    if (selectedObject == nullptr)
    {
        activeField = ActiveField::None;
        inputBuffer.clear();
        return;
    }

    // Sprite path editing is handled separately.
    if (activeField == ActiveField::SpritePath)
    {
        if (!inputBuffer.empty())
        {
            bool loaded =
                selectedObject->SetSprite(inputBuffer);

            if (!loaded)
            {
                TraceLog(
                    LOG_WARNING,
                    "Inspector: Failed to load sprite: %s",
                    inputBuffer.c_str()
                );
            }
        }

        activeField = ActiveField::None;
        inputBuffer.clear();

        return;
    }

    if (inputBuffer.empty())
    {
        activeField = ActiveField::None;
        inputBuffer.clear();
        return;
    }

    try
    {
        float value = std::stof(inputBuffer);

        TransformComponent& transform =
            selectedObject->GetTransform();

        switch (activeField)
        {
            case ActiveField::PositionX:
            {
                Vector2 position =
                    transform.GetPosition();

                position.x = value;

                transform.SetPosition(position);

                break;
            }

            case ActiveField::PositionY:
            {
                Vector2 position =
                    transform.GetPosition();

                position.y = value;

                transform.SetPosition(position);

                break;
            }

            case ActiveField::Rotation:
            {
                transform.SetRotation(value);

                break;
            }

            case ActiveField::ScaleX:
            {
                Vector2 scale =
                    transform.GetScale();

                scale.x = value;

                transform.SetScale(scale);

                break;
            }

            case ActiveField::ScaleY:
            {
                Vector2 scale =
                    transform.GetScale();

                scale.y = value;

                transform.SetScale(scale);

                break;
            }

            case ActiveField::SpritePath:
            case ActiveField::None:
            {
                break;
            }
        }
    }
    catch (...)
    {
        TraceLog(
            LOG_WARNING,
            "Inspector: Invalid numeric value."
        );
    }

    activeField = ActiveField::None;
    inputBuffer.clear();
}

// --------------------------------------------------
// Input Handling
// --------------------------------------------------

void Inspector::AddCharacterToInput()
{
    int key = GetCharPressed();

    while (key > 0)
    {
        if (activeField == ActiveField::SpritePath)
        {
            // Allow normal file path characters.
            if (key >= 32 && key <= 126)
            {
                inputBuffer += static_cast<char>(key);
            }
        }
        else
        {
            if (key >= '0' && key <= '9')
            {
                inputBuffer += static_cast<char>(key);
            }
            else if (key == '.')
            {
                if (inputBuffer.find('.') == std::string::npos)
                {
                    inputBuffer += '.';
                }
            }
            else if (key == '-')
            {
                if (inputBuffer.empty())
                {
                    inputBuffer += '-';
                }
            }
        }

        key = GetCharPressed();
    }
}

// --------------------------------------------------
// Mouse Over
// --------------------------------------------------

bool Inspector::IsMouseOver() const
{
    return CheckCollisionPointRec(
        GetMousePosition(),
        panel
    );
}
