#include "../../Engine/Core/InputManager.h"

#include "Inspector.h"
#include "../FileDialog.h"
#include "../../Engine/Animation/AnimationController.h"
#include "../../Engine/Physics/BoxCollider2D.h"
#include "../../Engine/Physics/Rigidbody2D.h"
#include "../../Engine/Audio/AudioSource.h"
#include "../../Engine/Audio/AudioListener.h"
#include "../../Engine/Rendering/ParticleEmitter.h"
#include "../../Engine/Rendering/PointLight2D.h"
#include "../../Engine/UI/UIComponent.h"
#include "raylib.h"
#include "rlgl.h"

#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <string>

// --------------------------------------------------
// Constructor
// --------------------------------------------------

Inspector::Inspector(Scene& sceneReference)
    : scene(sceneReference)
{
    width = 300.0f;
    height = static_cast<float>(GetScreenHeight()) - 27.0f;

    panel =
    {
        static_cast<float>(GetScreenWidth()) - width,
        0.0f,
        width,
        height
    };

    activeField = ActiveField::None;
    animationTab = false;
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
    if (!visible) return;
    width = 300.0f;
    height = static_cast<float>(GetScreenHeight()) - 27.0f;

    panel.x = static_cast<float>(GetScreenWidth()) - width;
    panel.y = 0.0f;
    panel.width = width;
    panel.height = height;

    Vector2 mousePosition = InputManager::GetInstance().GetMousePosition();
    mousePosition.y -= 27.0f;

    GameObject* selectedObject = scene.GetSelectedObject();

    // Continue keyboard editing even if the mouse moves
    // outside the Inspector panel.
    if (activeField != ActiveField::None)
    {
        AddCharacterToInput();

        if (InputManager::GetInstance().IsKeyPressed(KEY_BACKSPACE))
        {
            if (!inputBuffer.empty())
            {
                inputBuffer.pop_back();
            }
        }

        if (InputManager::GetInstance().IsKeyPressed(KEY_ENTER))
        {
            CommitEditing();
        }

        if (InputManager::GetInstance().IsKeyPressed(KEY_ESCAPE))
        {
            activeField = ActiveField::None;
            inputBuffer.clear();
            editingComponent = nullptr;
        }
    }
    if (addComponentMenuOpen && componentSearchFocused)
    {
        int character = InputManager::GetInstance().GetCharPressed();
        while (character > 0)
        {
            if (character >= 32 && character <= 126) { componentSearch.push_back(static_cast<char>(character)); componentMenuScroll = 0; }
            character = InputManager::GetInstance().GetCharPressed();
        }
        if (InputManager::GetInstance().IsKeyPressed(KEY_BACKSPACE) && !componentSearch.empty()) { componentSearch.pop_back(); componentMenuScroll = 0; }
        if (InputManager::GetInstance().IsKeyPressed(KEY_ESCAPE)) { addComponentMenuOpen = false; componentSearchFocused = false; }
    }

    if (selectedObject == nullptr)
    {
        return;
    }

    if (!CheckCollisionPointRec(mousePosition, panel))
    {
        return;
    }

    if (InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        const Rectangle nameField{panel.x + 15.0f, 52.0f, 205.0f, 27.0f};
        const Rectangle activeToggle{panel.x + 235.0f, 52.0f, 50.0f, 27.0f};
        if (CheckCollisionPointRec(mousePosition, activeToggle))
        {
            CommitEditing();
            selectedObject->SetActive(!selectedObject->IsActive());
            return;
        }
        if (CheckCollisionPointRec(mousePosition, nameField))
        {
            CommitEditing();
            activeField = ActiveField::GameObjectName;
            inputBuffer = selectedObject->GetName();
            return;
        }
        const Rectangle objectTab = {panel.x + 155.0f, 12.0f, 65.0f, 25.0f};
        const Rectangle animationTabButton = {panel.x + 225.0f, 12.0f, 65.0f, 25.0f};
        if (CheckCollisionPointRec(mousePosition, objectTab))
        {
            CommitEditing();
            animationTab = false;
            return;
        }
        if (CheckCollisionPointRec(mousePosition, animationTabButton))
        {
            CommitEditing();
            animationTab = true;
            return;
        }
    }

    if (animationTab)
    {
        UpdateAnimationEditor(selectedObject, mousePosition);
        return;
    }

    UpdateComponentStack(selectedObject, mousePosition);
    if (activeField == ActiveField::ComponentText) return;
    if (addComponentMenuOpen && mousePosition.x >= panel.x && mousePosition.y >= 700.0f) return;

    if (selectedObject->GetComponent<RectTransformComponent>())
    {
        UpdateRectTransform(selectedObject, mousePosition);
        if (selectedObject->GetComponent<UIComponent>() != nullptr || selectedObject->GetComponent<UICanvas>() != nullptr) return;
    }

    // --------------------------------------------------
    // Mouse interaction
    // --------------------------------------------------

    if (!InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
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
        const std::string path = FileDialog::OpenFile("Select Sprite Texture",
            "Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.tga)|*.png;*.jpg;*.jpeg;*.bmp;*.tga|All Files (*.*)|*.*|",
            "Assets", GetWindowHandle());
        if (!path.empty()) selectedObject->SetSprite(path);

        return;
    }

    // --------------------------------------------------
    // Sprite tint presets
    // --------------------------------------------------

    SpriteRenderer& sprite =
        selectedObject->GetSpriteRenderer();

    if (mousePosition.y >= 510.0f &&
        mousePosition.y <= 535.0f)
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
        570.0f,
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
    if (!visible) return;
    rlPushMatrix();
    rlTranslatef(0.0f, 27.0f, 0.0f);
    DrawRectangleRec(panel, DARKGRAY);

    DrawText(
        "INSPECTOR",
        static_cast<int>(panel.x + 15.0f),
        15,
        20,
        WHITE
    );

    const Rectangle objectTab = {panel.x + 155.0f, 12.0f, 65.0f, 25.0f};
    const Rectangle animationTabButton = {panel.x + 225.0f, 12.0f, 65.0f, 25.0f};
    DrawRectangleRec(objectTab, animationTab ? GRAY : SKYBLUE);
    DrawText("Object", static_cast<int>(objectTab.x + 9.0f), 18, 12, WHITE);
    DrawRectangleRec(animationTabButton, animationTab ? SKYBLUE : GRAY);
    DrawText("Animation", static_cast<int>(animationTabButton.x + 4.0f), 18, 11, WHITE);

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

        rlPopMatrix();
        return;
    }

    const Rectangle objectNameField{panel.x + 15.0f, 52.0f, 205.0f, 27.0f};
    DrawRectangleRec(objectNameField, activeField == ActiveField::GameObjectName ? LIGHTGRAY : Color{38, 39, 42, 255});
    const std::string displayedName = activeField == ActiveField::GameObjectName ? inputBuffer : selectedObject->GetName();
    DrawText(displayedName.c_str(), static_cast<int>(objectNameField.x + 7.0f), 58, 15,
        activeField == ActiveField::GameObjectName ? BLACK : WHITE);
    const Rectangle activeToggle{panel.x + 235.0f, 52.0f, 50.0f, 27.0f};
    DrawRectangleRec(activeToggle, selectedObject->IsActive() ? DARKGREEN : MAROON);
    DrawText(selectedObject->IsActive() ? "Active" : "Off", static_cast<int>(activeToggle.x + 7.0f), 59, 12, WHITE);

    std::string uiSummary;
    if (selectedObject->GetComponent<UICanvas>()) uiSummary += "Canvas  ";
    if (selectedObject->GetComponent<UITextComponent>()) uiSummary += "Text  ";
    if (selectedObject->GetComponent<UIProgressBar>()) uiSummary += "Progress Bar  ";
    if (selectedObject->GetComponent<UIButtonComponent>()) uiSummary += "Button  ";
    if (selectedObject->GetComponent<UIImageComponent>()) uiSummary += "Image  ";
    if (selectedObject->GetComponent<UIInputField>()) uiSummary += "Input Field  ";
    if (selectedObject->GetComponent<UILayoutGroup>()) uiSummary += "Layout Group  ";
    if (selectedObject->GetComponent<UIEventSystem>()) uiSummary += "Event System  ";
    if (!uiSummary.empty()) DrawText(TextFormat("UI: %s", uiSummary.c_str()), static_cast<int>(panel.x + 15.0f), 78, 12, SKYBLUE);

    if (animationTab)
    {
        DrawAnimationEditor(selectedObject);
        rlPopMatrix();
        return;
    }

    DrawTransform(selectedObject);
    if (selectedObject->GetComponent<RectTransformComponent>() != nullptr)
        DrawRectTransform(selectedObject);
    else
        DrawSprite(selectedObject);
    DrawComponentStack(selectedObject);
    rlPopMatrix();
}

void Inspector::DrawComponentStack(GameObject* object)
{
    if (object == nullptr) return;
    const float left = panel.x + 10.0f;
    const float rowTop = 660.0f;
    DrawRectangle(static_cast<int>(left), static_cast<int>(rowTop), static_cast<int>(width - 20.0f), 30, GRAY);
    DrawText("Components", static_cast<int>(left + 9.0f), static_cast<int>(rowTop + 6.0f), 16, WHITE);

    const float buttonY = rowTop + 36.0f;
    const Rectangle addButton{left, buttonY, width - 20.0f, 29.0f};
    DrawRectangleRec(addButton, addComponentMenuOpen ? DARKBLUE : DARKGRAY);
    DrawText(addComponentMenuOpen ? "Close Add Component" : "+ Add Component",
        static_cast<int>(addButton.x + 9), static_cast<int>(addButton.y + 7), 13, WHITE);

    float contentHeight = 0.0f;
    for (const auto& component : object->GetComponents())
        if (component) contentHeight += 33.0f + (collapsedComponents.count(component.get()) ? 0.0f : static_cast<float>(GetComponentProperties(component.get()).size()) * 25.0f);
    const float viewportHeight = std::max(0.0f, panel.height - (buttonY + 42.0f));
    const float maxComponentScroll = std::max(0.0f, contentHeight - viewportHeight);
    componentScroll = std::clamp(componentScroll, 0.0f, maxComponentScroll);
    float y = buttonY + 36.0f - componentScroll;
    for (const auto& component : object->GetComponents())
    {
        if (component == nullptr) continue;
        if (y + 31.0f >= buttonY + 36.0f && y < panel.height - 8.0f)
        {
            const Rectangle row{left, y, width - 20.0f, 29.0f};
            DrawRectangleRec(row, Color{57, 59, 63, 255});
            DrawRectangle(static_cast<int>(row.x), static_cast<int>(row.y), 3, 29,
                component->IsEnabled() ? SKYBLUE : GRAY);
            const Rectangle enabledBox{row.x + 9.0f, row.y + 7.0f, 15.0f, 15.0f};
            DrawRectangleRec(enabledBox, component->IsEnabled() ? SKYBLUE : DARKGRAY);
            if (component->IsEnabled()) DrawText("v", static_cast<int>(enabledBox.x + 3), static_cast<int>(enabledBox.y), 12, WHITE);
            const char* name = "Component";
            if (dynamic_cast<AnimationComponent*>(component.get())) name = "Animation";
            else if (dynamic_cast<AnimationController*>(component.get())) name = "Animation Controller";
            else if (dynamic_cast<BoxCollider2D*>(component.get())) name = "Box Collider 2D";
            else if (dynamic_cast<Rigidbody2D*>(component.get())) name = "Rigidbody 2D";
            else if (dynamic_cast<AudioSource*>(component.get())) name = "Audio Source";
            else if (dynamic_cast<AudioListener*>(component.get())) name = "Audio Listener";
            else if (dynamic_cast<ParticleEmitter*>(component.get())) name = "Particle Emitter";
            else if (dynamic_cast<PointLight2D*>(component.get())) name = "Point Light 2D";
            else if (dynamic_cast<UICanvas*>(component.get())) name = "Canvas";
            else if (dynamic_cast<UIEventSystem*>(component.get())) name = "Event System";
            else if (dynamic_cast<RectTransformComponent*>(component.get())) name = "Rect Transform";
            else if (dynamic_cast<UIImageComponent*>(component.get())) name = "Image";
            else if (dynamic_cast<UITextComponent*>(component.get())) name = "Text";
            else if (dynamic_cast<UIButtonComponent*>(component.get())) name = "Button";
            else if (dynamic_cast<UIInputField*>(component.get())) name = "Input Field";
            else if (dynamic_cast<UIProgressBar*>(component.get())) name = "Progress Bar";
            else if (dynamic_cast<UILayoutGroup*>(component.get())) name = "Layout Group";
            DrawText(name, static_cast<int>(row.x + 34.0f), static_cast<int>(row.y + 8.0f), 13,
                component->IsEnabled() ? WHITE : LIGHTGRAY);
            DrawText(collapsedComponents.count(component.get()) ? ">" : "v", static_cast<int>(row.x + row.width - 48.0f), static_cast<int>(row.y + 7.0f), 14, LIGHTGRAY);
            DrawText("x", static_cast<int>(row.x + row.width - 21.0f), static_cast<int>(row.y + 8.0f), 14, WHITE);
        }
        y += 33.0f;
        if (!collapsedComponents.count(component.get()))
        {
            const auto properties = GetComponentProperties(component.get());
            for (std::size_t propertyIndex = 0; propertyIndex < properties.size(); ++propertyIndex)
            {
                const ComponentProperty& property = properties[propertyIndex];
                if (y + 24.0f >= buttonY + 36.0f && y < panel.height - 8.0f)
                    DrawComponentProperty(component.get(), propertyIndex, property, y);
                y += 25.0f;
            }
        }
    }

    if (!addComponentMenuOpen) return;
    const float menuY = buttonY + 32.0f;
    const float menuHeight = panel.height - menuY - 8.0f;
    if (menuHeight <= 35.0f) return;
    DrawRectangle(static_cast<int>(left), static_cast<int>(menuY), static_cast<int>(width - 20.0f), static_cast<int>(menuHeight), Color{39, 40, 43, 255});
    DrawRectangleLines(static_cast<int>(left), static_cast<int>(menuY), static_cast<int>(width - 20.0f), static_cast<int>(menuHeight), DARKGRAY);
    const Rectangle search{left + 6.0f, menuY + 6.0f, width - 32.0f, 25.0f};
    DrawRectangleRec(search, BLACK);
    DrawText((componentSearch.empty() ? "Search components..." : componentSearch.c_str()),
        static_cast<int>(search.x + 6.0f), static_cast<int>(search.y + 6.0f), 12,
        componentSearch.empty() ? GRAY : WHITE);

    struct Entry { const char* title; const char* category; int id; };
    static const Entry entries[] = {
        {"Animation", "Animation", 0}, {"Animation Controller", "Animation", 1},
        {"Audio Source", "Audio", 2}, {"Audio Listener", "Audio", 3},
        {"Box Collider 2D", "Physics 2D", 4}, {"Rigidbody 2D", "Physics 2D", 5},
        {"Particle Emitter", "Effects", 6}, {"Point Light 2D", "Rendering", 7},
        {"Canvas", "UI", 8}, {"Event System", "UI", 9}, {"Rect Transform", "UI", 10},
        {"Image", "UI", 11}, {"Text", "UI", 12}, {"Button", "UI", 13},
        {"Input Field", "UI", 14}, {"Progress Bar", "UI", 15}, {"Layout Group", "Layout", 16}
    };
    const float listY = menuY + 37.0f;
    int visibleIndex = 0;
    int maxRows = static_cast<int>((menuHeight - 42.0f) / 30.0f);
    if (maxRows < 1) maxRows = 1;
    const int firstVisible = componentMenuScroll;
    const int lastVisible = firstVisible + maxRows;
    int matchIndex = 0;
    std::string query = componentSearch;
    for (char& c : query) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    for (const Entry& entry : entries)
    {
        std::string candidate = std::string(entry.title) + " " + entry.category;
        for (char& c : candidate) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (!query.empty() && candidate.find(query) == std::string::npos) continue;
        const float yRow = listY + (visibleIndex - firstVisible) * 30.0f;
        if (visibleIndex >= firstVisible && visibleIndex < lastVisible)
        {
            DrawText(entry.category, static_cast<int>(left + 8.0f), static_cast<int>(yRow), 10, SKYBLUE);
            DrawText(entry.title, static_cast<int>(left + 8.0f), static_cast<int>(yRow + 11.0f), 13, WHITE);
        }
        ++visibleIndex;
        ++matchIndex;
    }
}

std::vector<Inspector::ComponentProperty> Inspector::GetComponentProperties(Component* component) const
{
    std::vector<ComponentProperty> properties;
    auto number = [&properties](const char* label, float value) {
        properties.push_back({label, TextFormat("%.2f", value), false, false});
    };
    auto toggle = [&properties](const char* label, bool value) {
        properties.push_back({label, value ? "On" : "Off", true, value});
    };
    if (auto* collider = dynamic_cast<BoxCollider2D*>(component))
    {
        const Vector2 size = collider->GetSize(), offset = collider->GetOffset();
        number("Size X", size.x); number("Size Y", size.y); number("Offset X", offset.x); number("Offset Y", offset.y); toggle("Is Trigger", collider->IsTrigger());
    }
    else if (auto* body = dynamic_cast<Rigidbody2D*>(component))
    {
        const Vector2 velocity = body->GetVelocity(), acceleration = body->GetAcceleration();
        number("Mass", body->GetMass()); number("Velocity X", velocity.x); number("Velocity Y", velocity.y);
        number("Acceleration X", acceleration.x); number("Acceleration Y", acceleration.y);
        toggle("Use Gravity", body->IsGravityEnabled()); toggle("Is Kinematic", body->IsKinematic());
    }
    else if (auto* source = dynamic_cast<AudioSource*>(component))
    {
        properties.push_back({"Audio Clip", source->GetClipPath().empty() ? "None" : source->GetClipPath(), false, false, true});
        number("Volume", source->GetVolume()); number("Pitch", source->GetPitch()); toggle("Playing", source->IsPlaying());
    }
    else if (auto* emitter = dynamic_cast<ParticleEmitter*>(component))
    {
        number("Emission Rate", emitter->emissionRate); number("Lifetime", emitter->lifetime); number("Speed", emitter->speed); number("Size", emitter->particleSize);
        properties.push_back({"Particle Color", "Click to cycle", false, false});
    }
    else if (auto* light = dynamic_cast<PointLight2D*>(component))
    {
        number("Radius", light->radius); number("Intensity", light->intensity); properties.push_back({"Light Color", "Click to cycle", false, false});
    }
    else if (auto* canvas = dynamic_cast<UICanvas*>(component))
    {
        properties.push_back({"Render Mode", canvas->renderMode == UICanvas::RenderMode::ScreenSpaceOverlay ? "Screen Space Overlay" : (canvas->renderMode == UICanvas::RenderMode::ScreenSpaceCamera ? "Screen Space Camera" : "World Space"), false, false});
        toggle("Scale With Screen", canvas->scaleMode == UICanvas::ScaleMode::ScaleWithScreenSize);
        number("Reference Width", canvas->referenceResolution.x); number("Reference Height", canvas->referenceResolution.y);
        number("Match Width/Height", canvas->matchWidthOrHeight); number("Sorting Order", static_cast<float>(canvas->sortingOrder));
    }
    else if (auto* events = dynamic_cast<UIEventSystem*>(component)) toggle("Navigation Events", events->sendNavigationEvents);
    else if (auto* rect = dynamic_cast<RectTransformComponent*>(component))
    {
        number("Position X", rect->anchoredPosition.x); number("Position Y", rect->anchoredPosition.y);
        number("Width", rect->sizeDelta.x); number("Height", rect->sizeDelta.y);
        number("Anchor Min X", rect->anchorMin.x); number("Anchor Min Y", rect->anchorMin.y);
        number("Anchor Max X", rect->anchorMax.x); number("Anchor Max Y", rect->anchorMax.y);
    }
    else if (auto* image = dynamic_cast<UIImageComponent*>(component))
        properties.push_back({"Source", image->GetPath().empty() ? "None" : image->GetPath(), false, false, true});
    else if (auto* text = dynamic_cast<UITextComponent*>(component))
    {
        properties.push_back({"Text", text->text, false, false, true}); number("Font Size", static_cast<float>(text->fontSize));
    }
    else if (auto* button = dynamic_cast<UIButtonComponent*>(component))
    {
        properties.push_back({"Label", button->label, false, false, true});
    }
    else if (auto* input = dynamic_cast<UIInputField*>(component))
    {
        properties.push_back({"Text", input->text, false, false, true}); properties.push_back({"Placeholder", input->placeholder, false, false, true});
    }
    else if (auto* progress = dynamic_cast<UIProgressBar*>(component))
    {
        number("Value", progress->value); number("Maximum", progress->maximum);
    }
    else if (auto* layout = dynamic_cast<UILayoutGroup*>(component))
    {
        toggle("Horizontal", layout->direction == UILayoutGroup::Direction::Horizontal);
        number("Spacing", layout->spacing); number("Padding", layout->padding); number("Item Width", layout->itemSize.x); number("Item Height", layout->itemSize.y);
    }
    else if (auto* animation = dynamic_cast<AnimationComponent*>(component))
    {
        number("Frames Per Second", animation->GetFPS()); number("Frame Count", static_cast<float>(animation->GetFrameCount()));
        number("Current Frame", static_cast<float>(animation->GetCurrentFrame() + 1)); toggle("Loop", animation->IsLooping()); toggle("Playing", animation->IsPlaying());
    }
    else if (auto* controller = dynamic_cast<AnimationController*>(component))
        properties.push_back({"Current State", controller->GetCurrentState().empty() ? "None" : controller->GetCurrentState(), false, false, true});
    return properties;
}

void Inspector::DrawComponentProperty(Component* component, std::size_t propertyIndex, const ComponentProperty& property, float y) const
{
    const float left = panel.x + 10.0f;
    const Rectangle row{left, y, width - 20.0f, 23.0f};
    DrawRectangleRec(row, Color{48, 50, 53, 255});
    DrawText(property.label.c_str(), static_cast<int>(row.x + 9.0f), static_cast<int>(row.y + 5.0f), 11, LIGHTGRAY);
    if (property.toggle)
    {
        const Rectangle value{row.x + row.width - 72.0f, row.y + 2.0f, 62.0f, 19.0f};
        DrawRectangleRec(value, property.toggleValue ? DARKGREEN : DARKGRAY);
        DrawText(property.value.c_str(), static_cast<int>(value.x + 8.0f), static_cast<int>(value.y + 3.0f), 10, WHITE);
        return;
    }
    if (property.editableText)
    {
        const bool filePath = dynamic_cast<AudioSource*>(component) != nullptr ||
            dynamic_cast<UIImageComponent*>(component) != nullptr;
        const Rectangle value{row.x + row.width - 145.0f, row.y + 2.0f,
            filePath ? 83.0f : 135.0f, 19.0f};
        DrawRectangleRec(value, BLACK);
        std::string text = editingComponent == component && editingComponentProperty == propertyIndex && activeField == ActiveField::ComponentText
            ? inputBuffer : property.value;
        if (text.size() > 20) text = "..." + text.substr(text.size() - 17);
        DrawText(text.c_str(), static_cast<int>(value.x + 3.0f), static_cast<int>(value.y + 3.0f), 10, WHITE);
        if (filePath)
        {
            DrawRectangleRec({row.x + row.width - 57.0f, row.y + 2.0f, 47.0f, 19.0f}, Color{62, 83, 107, 255});
            DrawText("Browse", static_cast<int>(row.x + row.width - 54.0f), static_cast<int>(row.y + 6.0f), 9, WHITE);
        }
        return;
    }
    const Rectangle value{row.x + row.width - 112.0f, row.y + 2.0f, 70.0f, 19.0f};
    DrawText(property.value.c_str(), static_cast<int>(value.x + 2.0f), static_cast<int>(value.y + 3.0f), 10, WHITE);
    DrawRectangleRec({row.x + row.width - 38.0f, row.y + 2.0f, 16.0f, 19.0f}, DARKGRAY);
    DrawRectangleRec({row.x + row.width - 19.0f, row.y + 2.0f, 16.0f, 19.0f}, DARKGRAY);
    DrawText("-", static_cast<int>(row.x + row.width - 33.0f), static_cast<int>(row.y + 3.0f), 12, WHITE);
    DrawText("+", static_cast<int>(row.x + row.width - 15.0f), static_cast<int>(row.y + 3.0f), 12, WHITE);
}

void Inspector::ActivateComponentProperty(Component* component, std::size_t index)
{
    const float direction = InputManager::GetInstance().GetMousePosition().x >= panel.x + width - 29.0f ? 1.0f : -1.0f;
    auto change = [direction](float value, float step, float minimum, float maximum) { return std::clamp(value + direction * step, minimum, maximum); };
    if (auto* collider = dynamic_cast<BoxCollider2D*>(component))
    {
        Vector2 size = collider->GetSize(), offset = collider->GetOffset();
        if (index == 0) size.x = change(size.x, 1, 1, 100000); else if (index == 1) size.y = change(size.y, 1, 1, 100000);
        else if (index == 2) offset.x = change(offset.x, 1, -100000, 100000); else if (index == 3) offset.y = change(offset.y, 1, -100000, 100000); else { collider->SetTrigger(!collider->IsTrigger()); return; }
        collider->SetSize(size); collider->SetOffset(offset);
    }
    else if (auto* body = dynamic_cast<Rigidbody2D*>(component))
    {
        if (index == 0) body->SetMass(change(body->GetMass(), 0.1f, 0.01f, 100000));
        else if (index == 1 || index == 2) { Vector2 v = body->GetVelocity(); (index == 1 ? v.x : v.y) = change(index == 1 ? v.x : v.y, 1, -100000, 100000); body->SetVelocity(v); }
        else if (index == 3 || index == 4) { Vector2 a = body->GetAcceleration(); (index == 3 ? a.x : a.y) = change(index == 3 ? a.x : a.y, 1, -100000, 100000); body->SetAcceleration(a); }
        else if (index == 5) body->SetGravityEnabled(!body->IsGravityEnabled()); else body->SetKinematic(!body->IsKinematic());
    }
    else if (auto* source = dynamic_cast<AudioSource*>(component))
    {
        if (index == 1) source->SetVolume(change(source->GetVolume(), 0.05f, 0, 1));
        else if (index == 2) source->SetPitch(change(source->GetPitch(), 0.05f, 0.1f, 3));
        else if (index == 3) { if (source->IsPlaying()) source->Stop(); else source->Play(); }
    }
    else if (auto* emitter = dynamic_cast<ParticleEmitter*>(component))
    {
        if (index == 0) emitter->emissionRate = change(emitter->emissionRate, 1, 0, 10000);
        else if (index == 1) emitter->lifetime = change(emitter->lifetime, 0.1f, 0.05f, 1000);
        else if (index == 2) emitter->speed = change(emitter->speed, 5, 0, 10000);
        else if (index == 3) emitter->particleSize = change(emitter->particleSize, 1, 1, 256);
        else { const Color colors[] = {ORANGE, RED, YELLOW, SKYBLUE, WHITE}; int color = 0; for (int i = 0; i < 5; ++i) if (emitter->color.r == colors[i].r && emitter->color.g == colors[i].g && emitter->color.b == colors[i].b) color = i; emitter->color = colors[(color + (direction > 0 ? 1 : 4)) % 5]; }
    }
    else if (auto* light = dynamic_cast<PointLight2D*>(component))
    {
        if (index == 0) light->radius = change(light->radius, 10, 1, 100000);
        else if (index == 1) light->intensity = change(light->intensity, 0.1f, 0, 10);
        else { const Color colors[] = {WHITE, YELLOW, RED, BLUE, GREEN}; int color = 0; for (int i = 0; i < 5; ++i) if (light->color.r == colors[i].r && light->color.g == colors[i].g && light->color.b == colors[i].b) color = i; light->color = colors[(color + (direction > 0 ? 1 : 4)) % 5]; }
    }
    else if (auto* canvas = dynamic_cast<UICanvas*>(component))
    {
        if (index == 0) canvas->renderMode = static_cast<UICanvas::RenderMode>((static_cast<int>(canvas->renderMode) + (direction > 0 ? 1 : 2)) % 3);
        else if (index == 1) canvas->scaleMode = canvas->scaleMode == UICanvas::ScaleMode::ScaleWithScreenSize ? UICanvas::ScaleMode::ConstantPixelSize : UICanvas::ScaleMode::ScaleWithScreenSize;
        else if (index == 2) canvas->referenceResolution.x = change(canvas->referenceResolution.x, 16, 1, 16384);
        else if (index == 3) canvas->referenceResolution.y = change(canvas->referenceResolution.y, 16, 1, 16384);
        else if (index == 4) canvas->matchWidthOrHeight = change(canvas->matchWidthOrHeight, 0.1f, 0, 1);
        else canvas->sortingOrder += direction > 0 ? 1 : -1;
    }
    else if (auto* events = dynamic_cast<UIEventSystem*>(component)) events->sendNavigationEvents = !events->sendNavigationEvents;
    else if (auto* rect = dynamic_cast<RectTransformComponent*>(component))
    {
        if (index < 2) { (index == 0 ? rect->anchoredPosition.x : rect->anchoredPosition.y) += direction; }
        else if (index < 4) (index == 2 ? rect->sizeDelta.x : rect->sizeDelta.y) = change(index == 2 ? rect->sizeDelta.x : rect->sizeDelta.y, 1, 0, 100000);
        else if (index < 6) (index == 4 ? rect->anchorMin.x : rect->anchorMin.y) = change(index == 4 ? rect->anchorMin.x : rect->anchorMin.y, 0.05f, 0, 1);
        else (index == 6 ? rect->anchorMax.x : rect->anchorMax.y) = change(index == 6 ? rect->anchorMax.x : rect->anchorMax.y, 0.05f, 0, 1);
    }
    else if (auto* text = dynamic_cast<UITextComponent*>(component)) text->fontSize = static_cast<int>(change(static_cast<float>(text->fontSize), 1, 1, 256));
    else if (auto* progress = dynamic_cast<UIProgressBar*>(component))
    {
        if (index == 0) progress->value = change(progress->value, 1, 0, progress->maximum);
        else progress->maximum = change(progress->maximum, 1, 1, 100000);
    }
    else if (auto* layout = dynamic_cast<UILayoutGroup*>(component))
    {
        if (index == 0) layout->direction = layout->direction == UILayoutGroup::Direction::Horizontal ? UILayoutGroup::Direction::Vertical : UILayoutGroup::Direction::Horizontal;
        else if (index == 1) layout->spacing = change(layout->spacing, 1, 0, 10000);
        else if (index == 2) layout->padding = change(layout->padding, 1, 0, 10000);
        else if (index == 3) layout->itemSize.x = change(layout->itemSize.x, 1, 1, 100000);
        else layout->itemSize.y = change(layout->itemSize.y, 1, 1, 100000);
    }
    else if (auto* animation = dynamic_cast<AnimationComponent*>(component))
    {
        if (index == 0) animation->SetFPS(change(animation->GetFPS(), 1, 1, 120));
        else if (index == 1) animation->SetFrameCount(static_cast<int>(change(static_cast<float>(animation->GetFrameCount()), 1, 1, 10000)));
        else if (index == 2) animation->SetCurrentFrame(static_cast<int>(change(static_cast<float>(animation->GetCurrentFrame() + 1), 1, 1, static_cast<float>(std::max(1, animation->GetFrameCount())))) - 1);
        else if (index == 3) animation->SetLooping(!animation->IsLooping());
        else { if (animation->IsPlaying()) animation->Pause(); else animation->Play(); }
    }
}

void Inspector::UpdateComponentStack(GameObject* object, Vector2 mousePosition)
{
    if (object == nullptr) return;
    const float left = panel.x + 10.0f;
    const float buttonY = 696.0f;
    if (CheckCollisionPointRec(mousePosition, {left, 660.0f, width - 20.0f, 30.0f})) return;
    if (CheckCollisionPointRec(mousePosition, {left, buttonY, width - 20.0f, 29.0f}) &&
        InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (activeField != ActiveField::None) CommitEditing();
        addComponentMenuOpen = !addComponentMenuOpen;
        componentSearch.clear();
        componentMenuScroll = 0;
        componentSearchFocused = false;
        return;
    }
    if (mousePosition.x < left || mousePosition.x > panel.x + width - 10.0f) return;
    const float wheel = InputManager::GetInstance().GetMouseWheelMove();
    if (wheel != 0.0f && addComponentMenuOpen && mousePosition.y >= 728.0f)
        componentMenuScroll = std::max(0, componentMenuScroll - static_cast<int>(wheel));
    else if (wheel != 0.0f && mousePosition.y >= 732.0f)
        componentScroll = std::max(0.0f, componentScroll - wheel * 35.0f);

    if (!InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    if (addComponentMenuOpen)
    {
        const float menuY = 728.0f;
        const Rectangle search{left + 6.0f, menuY + 6.0f, width - 32.0f, 25.0f};
        if (CheckCollisionPointRec(mousePosition, search)) { componentSearchFocused = true; return; }
        componentSearchFocused = false;
    }
    if (!addComponentMenuOpen || mousePosition.y < 728.0f)
    {
        float y = 732.0f - componentScroll;
        for (const auto& component : object->GetComponents())
        {
            if (component == nullptr) continue;
            const Rectangle row{left, y, width - 20.0f, 29.0f};
            if (CheckCollisionPointRec(mousePosition, row))
            {
                if (mousePosition.x >= row.x + row.width - 28.0f)
                {
                    if (editingComponent == component.get()) { editingComponent = nullptr; activeField = ActiveField::None; inputBuffer.clear(); }
                    if (dynamic_cast<AnimationComponent*>(component.get())) object->RemoveComponent<AnimationComponent>();
                    else if (dynamic_cast<AnimationController*>(component.get())) object->RemoveComponent<AnimationController>();
                    else if (dynamic_cast<BoxCollider2D*>(component.get())) object->RemoveComponent<BoxCollider2D>();
                    else if (dynamic_cast<Rigidbody2D*>(component.get())) object->RemoveComponent<Rigidbody2D>();
                    else if (dynamic_cast<AudioSource*>(component.get())) object->RemoveComponent<AudioSource>();
                    else if (dynamic_cast<AudioListener*>(component.get())) object->RemoveComponent<AudioListener>();
                    else if (dynamic_cast<ParticleEmitter*>(component.get())) object->RemoveComponent<ParticleEmitter>();
                    else if (dynamic_cast<PointLight2D*>(component.get())) object->RemoveComponent<PointLight2D>();
                    else if (dynamic_cast<UICanvas*>(component.get())) object->RemoveComponent<UICanvas>();
                    else if (dynamic_cast<UIEventSystem*>(component.get())) object->RemoveComponent<UIEventSystem>();
                    else if (dynamic_cast<RectTransformComponent*>(component.get())) object->RemoveComponent<RectTransformComponent>();
                    else if (dynamic_cast<UIImageComponent*>(component.get())) object->RemoveComponent<UIImageComponent>();
                    else if (dynamic_cast<UITextComponent*>(component.get())) object->RemoveComponent<UITextComponent>();
                    else if (dynamic_cast<UIButtonComponent*>(component.get())) object->RemoveComponent<UIButtonComponent>();
                    else if (dynamic_cast<UIInputField*>(component.get())) object->RemoveComponent<UIInputField>();
                    else if (dynamic_cast<UIProgressBar*>(component.get())) object->RemoveComponent<UIProgressBar>();
                    else if (dynamic_cast<UILayoutGroup*>(component.get())) object->RemoveComponent<UILayoutGroup>();
                }
                else if (mousePosition.x < row.x + 30.0f) component->SetEnabled(!component->IsEnabled());
                else if (mousePosition.x >= row.x + row.width - 58.0f)
                {
                    if (collapsedComponents.count(component.get())) collapsedComponents.erase(component.get());
                    else collapsedComponents.insert(component.get());
                }
                return;
            }
            y += 33.0f;
            if (!collapsedComponents.count(component.get()))
            {
                const auto properties = GetComponentProperties(component.get());
                for (std::size_t propertyIndex = 0; propertyIndex < properties.size(); ++propertyIndex)
                {
                    const Rectangle propertyRow{left, y, width - 20.0f, 24.0f};
                    if (CheckCollisionPointRec(mousePosition, propertyRow))
                    {
                        if (properties[propertyIndex].editableText)
                        {
                            CommitEditing();
                            if (auto* source = dynamic_cast<AudioSource*>(component.get()))
                            {
                                const std::string path = FileDialog::OpenFile("Select Audio Clip",
                                    "Audio Files (*.wav;*.ogg;*.mp3;*.flac)|*.wav;*.ogg;*.mp3;*.flac|All Files (*.*)|*.*|",
                                    "Assets", GetWindowHandle());
                                if (!path.empty()) source->SetClip(path);
                            }
                            else if (auto* image = dynamic_cast<UIImageComponent*>(component.get()))
                            {
                                const std::string path = FileDialog::OpenFile("Select UI Image",
                                    "Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.tga)|*.png;*.jpg;*.jpeg;*.bmp;*.tga|All Files (*.*)|*.*|",
                                    "Assets", GetWindowHandle());
                                if (!path.empty()) image->Load(path);
                            }
                            else
                            {
                                editingComponent = component.get();
                                editingComponentProperty = propertyIndex;
                                activeField = ActiveField::ComponentText;
                                inputBuffer = properties[propertyIndex].value == "None" ? "" : properties[propertyIndex].value;
                            }
                        }
                        else if (mousePosition.x >= propertyRow.x + propertyRow.width - 68.0f)
                            ActivateComponentProperty(component.get(), propertyIndex);
                        return;
                    }
                    y += 25.0f;
                }
            }
        }
    }

    if (!addComponentMenuOpen) return;
    const float menuY = 728.0f;
    const Rectangle search{left + 6.0f, menuY + 6.0f, width - 32.0f, 25.0f};
    if (CheckCollisionPointRec(mousePosition, search)) return;
    struct Entry { const char* title; int id; };
    static const Entry entries[] = {
        {"Animation", 0}, {"Animation Controller", 1}, {"Audio Source", 2}, {"Audio Listener", 3},
        {"Box Collider 2D", 4}, {"Rigidbody 2D", 5}, {"Particle Emitter", 6}, {"Point Light 2D", 7},
        {"Canvas", 8}, {"Event System", 9}, {"Rect Transform", 10}, {"Image", 11}, {"Text", 12},
        {"Button", 13}, {"Input Field", 14}, {"Progress Bar", 15}, {"Layout Group", 16}
    };
    std::string query = componentSearch;
    for (char& c : query) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    int visibleIndex = 0;
    const float menuHeight = panel.height - menuY - 8.0f;
    const int maxRows = std::max(1, static_cast<int>((menuHeight - 42.0f) / 30.0f));
    for (const Entry& entry : entries)
    {
        std::string candidate = entry.title;
        for (char& c : candidate) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (!query.empty() && candidate.find(query) == std::string::npos) continue;
        const Rectangle item{left, menuY + 37.0f + (visibleIndex - componentMenuScroll) * 30.0f, width - 20.0f, 30.0f};
        if (visibleIndex >= componentMenuScroll && visibleIndex < componentMenuScroll + maxRows && CheckCollisionPointRec(mousePosition, item))
        {
            const bool duplicate = [&]() {
                switch (entry.id) {
                    case 0: return object->GetComponent<AnimationComponent>() != nullptr;
                    case 1: return object->GetComponent<AnimationController>() != nullptr;
                    case 2: return object->GetComponent<AudioSource>() != nullptr;
                    case 3: return object->GetComponent<AudioListener>() != nullptr;
                    case 4: return object->GetComponent<BoxCollider2D>() != nullptr;
                    case 5: return object->GetComponent<Rigidbody2D>() != nullptr;
                    case 6: return object->GetComponent<ParticleEmitter>() != nullptr;
                    case 7: return object->GetComponent<PointLight2D>() != nullptr;
                    case 8: return object->GetComponent<UICanvas>() != nullptr;
                    case 9: return object->GetComponent<UIEventSystem>() != nullptr;
                    case 10: return object->GetComponent<RectTransformComponent>() != nullptr;
                    case 11: return object->GetComponent<UIImageComponent>() != nullptr;
                    case 12: return object->GetComponent<UITextComponent>() != nullptr;
                    case 13: return object->GetComponent<UIButtonComponent>() != nullptr;
                    case 14: return object->GetComponent<UIInputField>() != nullptr;
                    case 15: return object->GetComponent<UIProgressBar>() != nullptr;
                    case 16: return object->GetComponent<UILayoutGroup>() != nullptr;
                    default: return true;
                }
            }();
            if (!duplicate)
            {
                switch (entry.id) {
                    case 0: object->AddComponent<AnimationComponent>(); break;
                    case 1: object->AddComponent<AnimationController>(); break;
                    case 2: object->AddComponent<AudioSource>(); break;
                    case 3: object->AddComponent<AudioListener>(); break;
                    case 4: object->AddComponent<BoxCollider2D>(); break;
                    case 5: object->AddComponent<Rigidbody2D>(); break;
                    case 6: object->AddComponent<ParticleEmitter>(); break;
                    case 7: object->AddComponent<PointLight2D>(); break;
                    case 8: object->AddComponent<UICanvas>(); break;
                    case 9: object->AddComponent<UIEventSystem>(); break;
                    case 10: object->AddComponent<RectTransformComponent>(); break;
                    case 11: if (!object->GetComponent<RectTransformComponent>()) object->AddComponent<RectTransformComponent>(); object->AddComponent<UIImageComponent>(); break;
                    case 12: if (!object->GetComponent<RectTransformComponent>()) object->AddComponent<RectTransformComponent>(); object->AddComponent<UITextComponent>(); break;
                    case 13: if (!object->GetComponent<RectTransformComponent>()) object->AddComponent<RectTransformComponent>(); object->AddComponent<UIButtonComponent>(); break;
                    case 14: if (!object->GetComponent<RectTransformComponent>()) object->AddComponent<RectTransformComponent>(); object->AddComponent<UIInputField>(); break;
                    case 15: if (!object->GetComponent<RectTransformComponent>()) object->AddComponent<RectTransformComponent>(); object->AddComponent<UIProgressBar>(); break;
                    case 16: object->AddComponent<UILayoutGroup>(); break;
                }
            }
            addComponentMenuOpen = false;
            componentSearchFocused = false;
            componentSearch.clear();
            return;
        }
        ++visibleIndex;
    }
    const int totalMatches = visibleIndex;
    if (totalMatches > maxRows) componentMenuScroll = std::min(componentMenuScroll, totalMatches - maxRows);
}

void Inspector::DrawRectTransform(GameObject* object)
{
    RectTransformComponent* rect = object->GetComponent<RectTransformComponent>();
    if (rect == nullptr) return;
    const float x = panel.x + 10.0f;
    DrawRectangle(static_cast<int>(x), 370, static_cast<int>(width - 20.0f), 30, GRAY);
    DrawText("Rect Transform", static_cast<int>(x + 10), 376, 18, WHITE);
    DrawText(TextFormat("Anchors  Min %.2f, %.2f   Max %.2f, %.2f", rect->anchorMin.x, rect->anchorMin.y, rect->anchorMax.x, rect->anchorMax.y), static_cast<int>(x + 5), 414, 11, LIGHTGRAY);
    DrawText(TextFormat("Pivot %.2f, %.2f   Position %.0f, %.0f", rect->pivot.x, rect->pivot.y, rect->anchoredPosition.x, rect->anchoredPosition.y), static_cast<int>(x + 5), 435, 11, LIGHTGRAY);
    DrawText(TextFormat("Size %.0f x %.0f", rect->sizeDelta.x, rect->sizeDelta.y), static_cast<int>(x + 5), 456, 12, WHITE);
    const char* presets[] = {"Top Left", "Center", "Top Right", "Stretch Bottom"};
    for (int i = 0; i < 4; ++i)
    {
        const Rectangle button{x + 4.0f + i * 69.0f, 480.0f, 66.0f, 26.0f};
        DrawRectangleRec(button, DARKGRAY);
        DrawText(presets[i], static_cast<int>(button.x + 3), 487, 9, WHITE);
    }
    const Rectangle controls[] = {{x + 12,520,30,25},{x + 48,520,30,25},{x + 135,520,30,25},{x + 171,520,30,25}};
    const char* labels[] = {"W-","W+","H-","H+"};
    for (int i = 0; i < 4; ++i) { DrawRectangleRec(controls[i], GRAY); DrawText(labels[i], (int)controls[i].x + 5, 527, 11, WHITE); }
    DrawText("Resize RectTransform", static_cast<int>(x + 220), 527, 10, LIGHTGRAY);
    if (const UICanvas* canvas = object->GetComponent<UICanvas>())
    {
        DrawText(TextFormat("Canvas: Screen Space Overlay | Sort Order %d", canvas->sortingOrder), static_cast<int>(x + 5), 565, 10, SKYBLUE);
        const Rectangle scalerButton{x + 5, 590, width - 30, 25};
        DrawRectangleRec(scalerButton, DARKGRAY);
        DrawText(canvas->scaleMode == UICanvas::ScaleMode::ScaleWithScreenSize ? "UI Scale Mode: Scale With Screen Size" : "UI Scale Mode: Constant Pixel Size",
            static_cast<int>(scalerButton.x + 5), static_cast<int>(scalerButton.y + 7), 10, WHITE);
        DrawText(TextFormat("Reference: %.0f x %.0f | Match W/H: %.1f", canvas->referenceResolution.x,
            canvas->referenceResolution.y, canvas->matchWidthOrHeight), static_cast<int>(x + 5), 620, 10, LIGHTGRAY);
        DrawText("Scale match: click to cycle 0 / 0.5 / 1", static_cast<int>(x + 5), 640, 10, LIGHTGRAY);
    }
    if (const UIImageComponent* image = object->GetComponent<UIImageComponent>())
        DrawText(TextFormat("Image Sprite: %.34s", image->IsLoaded() ? image->GetPath().c_str() : "None (drop an asset here)"), static_cast<int>(x + 5), 565, 10, LIGHTGRAY);
    if (const UITextComponent* text = object->GetComponent<UITextComponent>())
        DrawText(TextFormat("Text: %.38s | Size: %d", text->text.c_str(), text->fontSize), static_cast<int>(x + 5), 565, 10, LIGHTGRAY);
    if (const UIButtonComponent* button = object->GetComponent<UIButtonComponent>())
        DrawText(TextFormat("Button: %.38s | Hover: %s", button->label.c_str(), button->hovered ? "Yes" : "No"), static_cast<int>(x + 5), 565, 10, LIGHTGRAY);
    if (const UIInputField* input = object->GetComponent<UIInputField>())
        DrawText(TextFormat("Input Field: %.38s", input->text.c_str()), static_cast<int>(x + 5), 565, 10, LIGHTGRAY);
}

void Inspector::UpdateRectTransform(GameObject* object, Vector2 mousePosition)
{
    if (!InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    RectTransformComponent* rect = object->GetComponent<RectTransformComponent>();
    if (rect == nullptr) return;
    const float x = panel.x + 10.0f;
    if (UICanvas* canvas = object->GetComponent<UICanvas>())
    {
        if (CheckCollisionPointRec(mousePosition, {x + 5, 590, width - 30, 25}))
        {
            canvas->scaleMode = canvas->scaleMode == UICanvas::ScaleMode::ConstantPixelSize
                ? UICanvas::ScaleMode::ScaleWithScreenSize : UICanvas::ScaleMode::ConstantPixelSize;
            canvas->scaleWithScreen = canvas->scaleMode == UICanvas::ScaleMode::ScaleWithScreenSize;
            return;
        }
        if (CheckCollisionPointRec(mousePosition, {x + 5, 620, width - 30, 36}))
        {
            canvas->matchWidthOrHeight = canvas->matchWidthOrHeight >= 0.99f ? 0.0f :
                (canvas->matchWidthOrHeight < 0.25f ? 0.5f : 1.0f);
            return;
        }
    }
    const Vector2 anchors[] = {{0,0},{0.5f,0.5f},{1,0}};
    for (int i = 0; i < 3; ++i)
    {
        const Rectangle button{x + 4.0f + i * 69.0f, 480.0f, 66.0f, 26.0f};
        if (!CheckCollisionPointRec(mousePosition, button)) continue;
        rect->anchorMin = anchors[i]; rect->anchorMax = anchors[i]; rect->pivot = anchors[i]; return;
    }
    if (CheckCollisionPointRec(mousePosition, {x + 211.0f,480,76,26}))
    { rect->anchorMin = {0,1}; rect->anchorMax = {1,1}; rect->pivot = {0.5f,1}; return; }
    if (CheckCollisionPointRec(mousePosition, {x + 12,520,30,25})) rect->sizeDelta.x = std::max(1.0f, rect->sizeDelta.x - 10.0f);
    else if (CheckCollisionPointRec(mousePosition, {x + 48,520,30,25})) rect->sizeDelta.x += 10.0f;
    else if (CheckCollisionPointRec(mousePosition, {x + 135,520,30,25})) rect->sizeDelta.y = std::max(1.0f, rect->sizeDelta.y - 10.0f);
    else if (CheckCollisionPointRec(mousePosition, {x + 171,520,30,25})) rect->sizeDelta.y += 10.0f;
}

void Inspector::UpdateAnimationEditor(GameObject* object, Vector2 mousePosition)
{
    if (object == nullptr || !InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    AnimationComponent* animation = object->GetComponent<AnimationComponent>();
    AnimationController* controller = object->GetComponent<AnimationController>();
    if (animation == nullptr) return;

    const std::vector<AnimationClipInfo> clips = animation->GetAnimations();
    for (std::size_t index = 0; index < clips.size(); ++index)
    {
        const int row = static_cast<int>(index) / 2;
        const int column = static_cast<int>(index) % 2;
        const Rectangle button = {
            panel.x + 15.0f + column * 135.0f,
            145.0f + row * 36.0f,
            125.0f,
            28.0f
        };
        if (CheckCollisionPointRec(mousePosition, button))
        {
            animation->SetEnabled(true);
            if (controller != nullptr)
            {
                controller->SetEnabled(true);
                controller->PlayState(clips[index].name);
            }
            else animation->Play(clips[index].name, false);
            return;
        }
    }

    const Rectangle fpsMinus = {panel.x + 125.0f, 270.0f, 34.0f, 26.0f};
    const Rectangle fpsPlus = {panel.x + 238.0f, 270.0f, 34.0f, 26.0f};
    const Rectangle framesMinus = {panel.x + 125.0f, 308.0f, 34.0f, 26.0f};
    const Rectangle framesPlus = {panel.x + 238.0f, 308.0f, 34.0f, 26.0f};
    if (CheckCollisionPointRec(mousePosition, fpsMinus)) { animation->SetFPS(animation->GetFPS() - 1.0f); return; }
    if (CheckCollisionPointRec(mousePosition, fpsPlus)) { animation->SetFPS(animation->GetFPS() + 1.0f); return; }
    if (CheckCollisionPointRec(mousePosition, framesMinus)) { animation->SetFrameCount(animation->GetFrameCount() - 1); return; }
    if (CheckCollisionPointRec(mousePosition, framesPlus)) { animation->SetFrameCount(animation->GetFrameCount() + 1); return; }

    const Rectangle loopButton = {panel.x + 15.0f, 347.0f, 130.0f, 28.0f};
    if (CheckCollisionPointRec(mousePosition, loopButton))
    {
        animation->SetLooping(!animation->IsLooping());
        return;
    }

    const Rectangle transportButtons[] = {
        {panel.x + 15.0f, 388.0f, 62.0f, 28.0f},
        {panel.x + 82.0f, 388.0f, 62.0f, 28.0f},
        {panel.x + 149.0f, 388.0f, 62.0f, 28.0f},
        {panel.x + 216.0f, 388.0f, 69.0f, 28.0f}
    };
    for (int index = 0; index < 4; ++index)
    {
        if (!CheckCollisionPointRec(mousePosition, transportButtons[index])) continue;
        if (index == 0) { animation->SetEnabled(true); animation->Play(); }
        else if (index == 1) animation->Pause();
        else if (index == 2) animation->Stop();
        else animation->Reset();
        return;
    }

    if (controller == nullptr) return;
    const Rectangle movingButton = {panel.x + 15.0f, 475.0f, 125.0f, 30.0f};
    const Rectangle runningButton = {panel.x + 150.0f, 475.0f, 135.0f, 30.0f};
    const Rectangle jumpButton = {panel.x + 15.0f, 517.0f, 125.0f, 30.0f};
    if (CheckCollisionPointRec(mousePosition, movingButton))
    {
        animation->SetEnabled(true);
        controller->SetEnabled(true);
        controller->SetBool("moving", !controller->GetBool("moving"));
    }
    else if (CheckCollisionPointRec(mousePosition, runningButton))
    {
        animation->SetEnabled(true);
        controller->SetEnabled(true);
        controller->SetBool("running", !controller->GetBool("running"));
    }
    else if (CheckCollisionPointRec(mousePosition, jumpButton))
    {
        animation->SetEnabled(true);
        controller->SetEnabled(true);
        controller->SetTrigger("jump");
    }
}

void Inspector::DrawAnimationEditor(GameObject* object)
{
    AnimationComponent* animation = object == nullptr ? nullptr : object->GetComponent<AnimationComponent>();
    AnimationController* controller = object == nullptr ? nullptr : object->GetComponent<AnimationController>();
    DrawText("Animation Editor", static_cast<int>(panel.x + 15.0f), 92, 20, WHITE);
    if (animation == nullptr || animation->GetAnimations().empty())
    {
        DrawText("No animation clips on this object.", static_cast<int>(panel.x + 15.0f), 130, 14, LIGHTGRAY);
        return;
    }

    DrawText("Named clips", static_cast<int>(panel.x + 15.0f), 121, 14, LIGHTGRAY);
    const std::vector<AnimationClipInfo> clips = animation->GetAnimations();
    for (std::size_t index = 0; index < clips.size(); ++index)
    {
        const int row = static_cast<int>(index) / 2;
        const int column = static_cast<int>(index) % 2;
        const Rectangle button = {
            panel.x + 15.0f + column * 135.0f,
            145.0f + row * 36.0f,
            125.0f,
            28.0f
        };
        const bool active = clips[index].name == animation->GetCurrentAnimationName();
        DrawRectangleRec(button, active ? DARKBLUE : GRAY);
        DrawText(clips[index].name.c_str(), static_cast<int>(button.x + 8.0f),
            static_cast<int>(button.y + 7.0f), 13, WHITE);
    }

    DrawText(TextFormat("Active: %s   Frame: %d", animation->GetCurrentAnimationName().c_str(), animation->GetCurrentFrame() + 1),
        static_cast<int>(panel.x + 15.0f), 238, 13, WHITE);
    DrawText("FPS", static_cast<int>(panel.x + 15.0f), 275, 14, WHITE);
    DrawRectangleRec({panel.x + 125.0f, 270.0f, 34.0f, 26.0f}, GRAY);
    DrawText("-", static_cast<int>(panel.x + 137.0f), 275, 16, WHITE);
    DrawText(TextFormat("%.1f", animation->GetFPS()), static_cast<int>(panel.x + 174.0f), 275, 14, WHITE);
    DrawRectangleRec({panel.x + 238.0f, 270.0f, 34.0f, 26.0f}, GRAY);
    DrawText("+", static_cast<int>(panel.x + 250.0f), 275, 16, WHITE);

    DrawText("Frames", static_cast<int>(panel.x + 15.0f), 313, 14, WHITE);
    DrawRectangleRec({panel.x + 125.0f, 308.0f, 34.0f, 26.0f}, GRAY);
    DrawText("-", static_cast<int>(panel.x + 137.0f), 313, 16, WHITE);
    DrawText(TextFormat("%d", animation->GetFrameCount()), static_cast<int>(panel.x + 184.0f), 313, 14, WHITE);
    DrawRectangleRec({panel.x + 238.0f, 308.0f, 34.0f, 26.0f}, GRAY);
    DrawText("+", static_cast<int>(panel.x + 250.0f), 313, 16, WHITE);

    const Rectangle loopButton = {panel.x + 15.0f, 347.0f, 130.0f, 28.0f};
    DrawRectangleRec(loopButton, animation->IsLooping() ? DARKGREEN : GRAY);
    DrawText(animation->IsLooping() ? "Looping: On" : "Looping: Off",
        static_cast<int>(loopButton.x + 8.0f), 354, 13, WHITE);

    const Color transportColor = GRAY;
    DrawRectangleRec({panel.x + 15.0f, 388.0f, 62.0f, 28.0f}, transportColor);
    DrawRectangleRec({panel.x + 82.0f, 388.0f, 62.0f, 28.0f}, transportColor);
    DrawRectangleRec({panel.x + 149.0f, 388.0f, 62.0f, 28.0f}, transportColor);
    DrawRectangleRec({panel.x + 216.0f, 388.0f, 69.0f, 28.0f}, transportColor);
    DrawText("Play", static_cast<int>(panel.x + 26.0f), 395, 13, WHITE);
    DrawText("Pause", static_cast<int>(panel.x + 89.0f), 395, 13, WHITE);
    DrawText("Stop", static_cast<int>(panel.x + 161.0f), 395, 13, WHITE);
    DrawText("Reset", static_cast<int>(panel.x + 225.0f), 395, 13, WHITE);

    DrawRectangle(static_cast<int>(panel.x + 10.0f), 435, static_cast<int>(width - 20.0f), 28, GRAY);
    DrawText("Animation Controller", static_cast<int>(panel.x + 18.0f), 441, 16, WHITE);
    if (controller == nullptr)
    {
        DrawText("No controller component", static_cast<int>(panel.x + 15.0f), 475, 13, LIGHTGRAY);
        return;
    }

    DrawText(TextFormat("State: %s", controller->GetCurrentState().c_str()),
        static_cast<int>(panel.x + 15.0f), 468, 14, WHITE);
    const Rectangle movingButton = {panel.x + 15.0f, 492.0f, 125.0f, 30.0f};
    const Rectangle runningButton = {panel.x + 150.0f, 492.0f, 135.0f, 30.0f};
    const Rectangle jumpButton = {panel.x + 15.0f, 534.0f, 125.0f, 30.0f};
    DrawRectangleRec(movingButton, controller->GetBool("moving") ? DARKGREEN : GRAY);
    DrawRectangleRec(runningButton, controller->GetBool("running") ? DARKGREEN : GRAY);
    DrawRectangleRec(jumpButton, DARKGRAY);
    DrawText(controller->GetBool("moving") ? "Moving: On" : "Moving: Off",
        static_cast<int>(movingButton.x + 8.0f), 500, 12, WHITE);
    DrawText(controller->GetBool("running") ? "Running: On" : "Running: Off",
        static_cast<int>(runningButton.x + 8.0f), 500, 12, WHITE);
    DrawText("Trigger Jump", static_cast<int>(jumpButton.x + 8.0f), 542, 12, WHITE);
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
        {pathField.x, pathField.y, pathField.width - 60.0f, pathField.height},
        activeField == ActiveField::SpritePath
            ? LIGHTGRAY
            : BLACK
    );

    const char* pathText =
        sprite.IsLoaded()
            ? sprite.GetFilePath().c_str()
            : "Select a texture";
    std::string displayPath = pathText;
    if (displayPath.size() > 30) displayPath = "..." + displayPath.substr(displayPath.size() - 27);

    DrawText(
        displayPath.c_str(),
        static_cast<int>(pathField.x + 5.0f),
        static_cast<int>(pathField.y + 7.0f),
        12,
        WHITE
    );
    DrawRectangleRec({pathField.x + pathField.width - 56.0f, pathField.y, 56.0f, pathField.height}, Color{62, 83, 107, 255});
    DrawText("Browse", static_cast<int>(pathField.x + pathField.width - 51.0f), static_cast<int>(pathField.y + 9.0f), 10, WHITE);

    DrawText(
        "Choose a file or drag one from the Project window",
        static_cast<int>(panel.x + 15.0f),
        470,
        11,
        LIGHTGRAY
    );

    DrawText(
        "Tint:",
        static_cast<int>(panel.x + 15.0f),
        485,
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
        510,
        14,
        WHITE
    );

    // Tint presets
    DrawRectangle(
        static_cast<int>(panel.x + 15.0f),
        510,
        35,
        25,
        WHITE
    );

    DrawRectangle(
        static_cast<int>(panel.x + 60.0f),
        510,
        35,
        25,
        RED
    );

    DrawRectangle(
        static_cast<int>(panel.x + 105.0f),
        510,
        35,
        25,
        GREEN
    );

    DrawRectangle(
        static_cast<int>(panel.x + 150.0f),
        510,
        35,
        25,
        BLUE
    );

    DrawRectangle(
        static_cast<int>(panel.x + 195.0f),
        510,
        35,
        25,
        YELLOW
    );

    DrawText(
        "Tint presets",
        static_cast<int>(panel.x + 15.0f),
        540,
        12,
        LIGHTGRAY
    );

    // Remove Sprite button
    DrawRectangle(
        static_cast<int>(panel.x + 15.0f),
        570,
        120,
        30,
        DARKGRAY
    );

    DrawText(
        "Remove Sprite",
        static_cast<int>(panel.x + 22.0f),
        578,
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
        editingComponent = nullptr;
        return;
    }

    // Sprite path editing is handled separately.
    if (activeField == ActiveField::SpritePath || activeField == ActiveField::ComponentText)
    {
        if (activeField == ActiveField::SpritePath && !inputBuffer.empty())
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
        else if (activeField == ActiveField::ComponentText && editingComponent != nullptr)
        {
            if (auto* source = dynamic_cast<AudioSource*>(editingComponent)) { if (inputBuffer.empty()) source->Unload(); else source->SetClip(inputBuffer); }
            else if (auto* image = dynamic_cast<UIImageComponent*>(editingComponent)) { if (inputBuffer.empty()) image->Unload(); else image->Load(inputBuffer); }
            else if (auto* text = dynamic_cast<UITextComponent*>(editingComponent)) text->text = inputBuffer;
            else if (auto* button = dynamic_cast<UIButtonComponent*>(editingComponent)) button->label = inputBuffer;
            else if (auto* input = dynamic_cast<UIInputField*>(editingComponent))
            {
                if (editingComponentProperty == 0) input->text = inputBuffer;
                else input->placeholder = inputBuffer;
            }
            else if (auto* controller = dynamic_cast<AnimationController*>(editingComponent)) controller->PlayState(inputBuffer);
        }

        activeField = ActiveField::None;
        inputBuffer.clear();
        editingComponent = nullptr;

        return;
    }
    if (activeField == ActiveField::GameObjectName)
    {
        if (!inputBuffer.empty()) selectedObject->SetName(inputBuffer);
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
            case ActiveField::ComponentText:
            case ActiveField::GameObjectName:
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
    int key = InputManager::GetInstance().GetCharPressed();

    while (key > 0)
    {
        if (activeField == ActiveField::SpritePath || activeField == ActiveField::ComponentText || activeField == ActiveField::GameObjectName)
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

        key = InputManager::GetInstance().GetCharPressed();
    }
}

// --------------------------------------------------
// Mouse Over
// --------------------------------------------------

bool Inspector::IsMouseOver() const
{
    if (!visible) return false;
    Vector2 mouse = InputManager::GetInstance().GetMousePosition();
    if (mouse.y < 27.0f) return false;
    mouse.y -= 27.0f;
    return CheckCollisionPointRec(
        mouse,
        panel
    );
}
