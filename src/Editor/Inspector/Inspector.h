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
    void SetVisible(bool value) { visible = value; }
    bool IsVisible() const { return visible; }

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
        ScaleY,
        SpritePath
    };

    ActiveField activeField;
    bool animationTab;
    bool visible = true;

    std::string inputBuffer;

    void DrawTransform(GameObject* object);

    void DrawSprite(GameObject* object);
    void DrawRectTransform(GameObject* object);
    void UpdateRectTransform(GameObject* object, Vector2 mousePosition);
    void DrawAnimationEditor(GameObject* object);
    void UpdateAnimationEditor(GameObject* object, Vector2 mousePosition);

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
