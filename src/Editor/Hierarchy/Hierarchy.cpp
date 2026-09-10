#include "Hierarchy.h"

Hierarchy::Hierarchy(Scene& sceneReference): scene(sceneReference){
    width = 250.0f;
    height = static_cast<float>(GetScreenHeight());

    selectedIndex = -1;

    panel =
    {
        0.0f,
        0.0f,
        width,
        height
    };

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
}

void Hierarchy::Initialize()
{
}

void Hierarchy::Update()
{
    height = static_cast<float>(GetScreenHeight());

    panel.height = height;

    createButton.y = height - 45.0f;

    deleteButton.y = height - 45.0f;

    if (IsKeyPressed(KEY_DELETE))
    {
        DeleteSelectedObject();
        return;
    }

    Vector2 mousePosition = GetMousePosition();

    if (!CheckCollisionPointRec(mousePosition,panel)){
        return;
    }


    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePosition,createButton)){
        CreateObject();
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePosition,deleteButton)){
        DeleteSelectedObject();
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        int objectHeight = 30;

        int index = static_cast<int>((mousePosition.y - 70.0f)/ objectHeight);

        std::vector<GameObject>& objects = scene.GetGameObjects();

        if (index >= 0 &&index < static_cast<int>(objects.size())){
            selectedIndex = index;
            scene.SelectObject(&objects[index]);
        }
    }
}

void Hierarchy::CreateObject()
{
    GameObject* object = scene.CreateGameObject();

    if (object == nullptr)
    {
        return;
    }

    std::vector<GameObject>& objects = scene.GetGameObjects();

    for (int i = 0;i < static_cast<int>(objects.size());i++){
        if (&objects[i] == object)
        {
            selectedIndex = i;
            break;
        }
    }
}

void Hierarchy::DeleteSelectedObject()
{
    GameObject* selectedObject = scene.GetSelectedObject();

    if (selectedObject == nullptr)
    {
        return;
    }

    scene.DestroyGameObject(selectedObject);

    selectedIndex = -1;
}

void Hierarchy::Draw()
{
    DrawRectangleRec(panel,DARKGRAY);

    DrawText("HIERARCHY",15,15,20,WHITE);

    DrawText("▼ Scene",15,45,18,WHITE);

    std::vector<GameObject>& objects = scene.GetGameObjects();


    int startY = 70;

    int objectHeight = 30;


    for (int i = 0;i < static_cast<int>(objects.size());i++){
        int y =startY +i * objectHeight;

        if (objects[i].IsSelected())
        {
            DrawRectangle(10,y,static_cast<int>(width - 20.0f),objectHeight,Fade(BLUE,0.4f));
        }

        DrawText(objects[i].GetName().c_str(),35,y + 5,18,WHITE);
    }

    DrawRectangleRec(createButton,GRAY);

    DrawText("+ Create",static_cast<int>(createButton.x + 15.0f),static_cast<int>(createButton.y + 6.0f),16,WHITE);

    DrawRectangleRec(deleteButton,GRAY);

    DrawText("- Delete",static_cast<int>(deleteButton.x + 15.0f),static_cast<int>(deleteButton.y + 6.0f),16,WHITE);
}

bool Hierarchy::IsMouseOver() const
{
    return CheckCollisionPointRec(GetMousePosition(),panel);
}