#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"

#include <string>

class Inspector
{
public:
    Inspector(Scene& scene);

    void Initialize();
    void Update();
    void Draw();

    bool IsMouseOver() const;

private:
    Scene& scene;

    Rectangle panel;

    float width;
    float height;

    enum class ActiveField
    {
        None,
        PositionX,
        PositionY,
        Rotation,
        ScaleX,
        ScaleY
    };

    ActiveField activeField;

    std::string inputBuffer;

    void DrawTransform(GameObject* object);

    void DrawField(
        const char* label,
        const char* value,
        Rectangle fieldRect,
        ActiveField field
    );

    void StartEditing(ActiveField field, float currentValue);
    void CommitEditing();

    void AddCharacterToInput();
};