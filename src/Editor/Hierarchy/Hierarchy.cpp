#include "Hierarchy.h"

Hierarchy::Hierarchy(Scene& sceneReference): scene(sceneReference){
    width = 250.0f;
    height = 720.0f;

    selectedIndex = -1;

    panal = {
        0.0f,
        0.0f,
        width,
        height
    };
}

void Hierarchy::Initialize()
{
}

void Hierarchy::Update()
{
    Vector2 mousePosition = GetMousePosition();

    if (!CheckCollisionPointRec(mousePosition, panal))
        return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        int objectHeight = 30;

        int index = (int)((mousePosition.y - 70.0f) / objectHeight);

        std::vector<GameObject>& objects = scene.GetGameObjects();

        if (index >= 0 && index < (int)objects.size()){
            selectedIndex = index;
            for (GameObject& object : objects)
            {
                object.SetSelected(false);
            }

            objects[index].SetSelected(true);
        }
    }
}

void Hierarchy::Draw()
{
    DrawRectangleRec(panal,DARKGRAY);

    DrawText("Hierarchy",15,15,20,WHITE);

    DrawText("▼ Scene",15,45,18,WHITE);


    std::vector<GameObject>& objects = scene.GetGameObjects();

    int startY = 70;
    int objectHeight = 30;

    for (int i = 0; i < (int)objects.size(); i++){
        int y = startY + i * objectHeight;

        if (objects[i].IsSelected())
        {
            DrawRectangle(10,y,width - 20,objectHeight,Fade(BLUE, 0.4f));
        }

        DrawText(
            TextFormat("GameObject %d", i + 1),35,y + 5,18,WHITE);
    }
}