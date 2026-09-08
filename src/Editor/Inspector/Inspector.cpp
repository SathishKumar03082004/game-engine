#include "Inspector.h"

Inspector::Inspector(Scene& sceneReference): scene(sceneReference){
    width = 300.0f;
    height = 720.0f;

    panel ={980.0f,0.0f,width,height};
}

void Inspector::Initialize()
{
}

void Inspector::Update()
{
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

void Inspector::DrawTransform(
    GameObject* object
)
{
    TransformComponent& transform =
        object->GetTransform();


    // =========================================================
    // TRANSFORM HEADER
    // =========================================================

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


    // =========================================================
    // POSITION
    // =========================================================

    DrawText(
        "Position",
        static_cast<int>(panel.x + 20.0f),
        145,
        18,
        WHITE
    );


    Vector2 position =
        transform.GetPosition();


    DrawText(
        TextFormat(
            "X: %.2f",
            position.x
        ),
        static_cast<int>(panel.x + 25.0f),
        175,
        17,
        LIGHTGRAY
    );


    DrawText(
        TextFormat(
            "Y: %.2f",
            position.y
        ),
        static_cast<int>(panel.x + 150.0f),
        175,
        17,
        LIGHTGRAY
    );


    // =========================================================
    // ROTATION
    // =========================================================

    DrawText(
        "Rotation",
        static_cast<int>(panel.x + 20.0f),
        215,
        18,
        WHITE
    );


    float rotation =
        transform.GetRotation();


    DrawText(
        TextFormat(
            "%.2f",
            rotation
        ),
        static_cast<int>(panel.x + 25.0f),
        245,
        17,
        LIGHTGRAY
    );

    DrawText(
        "Scale",
        static_cast<int>(panel.x + 20.0f),
        285,
        18,
        WHITE
    );


    Vector2 scale =
        transform.GetScale();


    DrawText(
        TextFormat(
            "X: %.2f",
            scale.x
        ),
        static_cast<int>(panel.x + 25.0f),
        315,
        17,
        LIGHTGRAY
    );


    DrawText(
        TextFormat(
            "Y: %.2f",
            scale.y
        ),
        static_cast<int>(panel.x + 150.0f),
        315,
        17,
        LIGHTGRAY
    );
}