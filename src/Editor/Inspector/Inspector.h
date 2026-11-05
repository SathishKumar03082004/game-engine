#pragma once

#include "raylib.h"

#include "../../Engine/Scene/Scene.h"

#include <string>
#include <unordered_set>
#include <vector>

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
        SpritePath,
        ComponentText,
        GameObjectName
    };

    ActiveField activeField;
    bool animationTab;
    bool visible = true;
    bool addComponentMenuOpen = false;
    std::string componentSearch;
    float componentScroll = 0.0f;
    int componentMenuScroll = 0;
    bool componentSearchFocused = false;
    std::unordered_set<const Component*> collapsedComponents;
    Component* editingComponent = nullptr;
    std::size_t editingComponentProperty = 0;

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
    void DrawComponentStack(GameObject* object);
    void UpdateComponentStack(GameObject* object, Vector2 mousePosition);
    struct ComponentProperty { std::string label; std::string value; bool toggle = false; bool toggleValue = false; bool editableText = false; };
    std::vector<ComponentProperty> GetComponentProperties(Component* component) const;
    void DrawComponentProperty(Component* component, std::size_t propertyIndex, const ComponentProperty& property, float y) const;
    void ActivateComponentProperty(Component* component, std::size_t propertyIndex);
};
