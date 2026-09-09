#include "Inspector.h"

#include <cstdlib>

Inspector::Inspector(Scene& sceneReference): scene(sceneReference){
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
    inputBuffer = "";
}

void Inspector::Initialize()
{
    
}


void Inspector::Update()
{
    width = 300.0f;
    height = static_cast<float>(GetScreenHeight());

    panel.x = static_cast<float>(GetScreenWidth()) - width;
    panel.y = 0.0f;
    panel.width = width;
    panel.height = height;

    Vector2 mousePosition = GetMousePosition();


    if (!CheckCollisionPointRec(mousePosition, panel))
    {
        return;
    }


    GameObject* selectedObject = scene.GetSelectedObject();

    if (selectedObject == nullptr)
    {
        return;
    }


    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        TransformComponent& transform = selectedObject->GetTransform();

        Vector2 position = transform.GetPosition();

        Vector2 scale = transform.GetScale();

        float rotation = transform.GetRotation();

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

            StartEditing(ActiveField::PositionY,position.y);
            return;
        }


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

            StartEditing(ActiveField::Rotation,rotation);
            return;
        }

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
            StartEditing(ActiveField::ScaleX,scale.x);

            return;
        }

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
            StartEditing(ActiveField::ScaleY,scale.y);
            return;
        }

        CommitEditing();
    }


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
}


void Inspector::Draw()
{
    DrawRectangleRec(panel,DARKGRAY);

    DrawText("INSPECTOR",static_cast<int>(panel.x + 15.0f),15,20,WHITE);


    GameObject* selectedObject = scene.GetSelectedObject();


    if (selectedObject == nullptr)
    {
        DrawText("Nothing Selected",static_cast<int>(panel.x + 15.0f),60,18,LIGHTGRAY);
        return;
    }


    DrawText(selectedObject->GetName().c_str(),static_cast<int>(panel.x + 15.0f),55,22,WHITE);

    DrawTransform(selectedObject);
}


void Inspector::DrawTransform(GameObject* object)
{
    TransformComponent& transform =
        object->GetTransform();

    Vector2 position =
        transform.GetPosition();

    float rotation =
        transform.GetRotation();

    Vector2 scale =
        transform.GetScale();


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


void Inspector::DrawField(const char* label,const char* value,Rectangle fieldRect,ActiveField field)
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


    bool active =
        activeField == field;

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


void Inspector::StartEditing(
    ActiveField field,
    float currentValue
)
{
    activeField = field;

    inputBuffer =
        TextFormat("%.2f", currentValue);
}


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


    if (inputBuffer.empty())
    {
        activeField = ActiveField::None;
        inputBuffer.clear();
        return;
    }


    try
    {
        float value =
            std::stof(inputBuffer);

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


            case ActiveField::None:
                break;
        }
    }
    catch (...)
    {
    }


    activeField = ActiveField::None;
    inputBuffer.clear();
}


void Inspector::AddCharacterToInput()
{
    int key = GetCharPressed();

    while (key > 0)
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
        key = GetCharPressed();
    }
}

bool Inspector::IsMouseOver() const
{
    return CheckCollisionPointRec(GetMousePosition(),panel);
}