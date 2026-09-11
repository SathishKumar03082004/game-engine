#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"

class Hierarchy
{
public:
    Hierarchy(Scene& sceneReference);

    void Initialize();
    void Update();
    void Draw();

    bool IsMouseOver() const;

private:
    Scene& scene;

    Rectangle panel;

    float width;
    float height;

    int selectedIndex;

    Rectangle createButton;
    Rectangle deleteButton;

    void CreateObject();
    void DeleteSelectedObject();

    void DrawObject(
        GameObject* object,
        int depth,
        float& y
    );

    void SelectObject(
        GameObject* object
    );
};