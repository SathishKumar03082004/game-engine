#include "../../Engine/Core/InputManager.h"

#include "Inspector.h"
#include "../../Engine/Animation/AnimationController.h"
#include "../../Engine/Physics/BoxCollider2D.h"
#include "../../Engine/Physics/Rigidbody2D.h"
#include "../../Engine/UI/UIComponent.h"
#include "raylib.h"
#include "rlgl.h"

#include <cstdlib>
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

    if (InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
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

    const Rectangle colliderButton = {panel.x + 15.0f, 650.0f, 170.0f, 25.0f};
    if (CheckCollisionPointRec(mousePosition, colliderButton))
    {
        if (selectedObject->GetComponent<BoxCollider2D>() != nullptr)
            selectedObject->RemoveComponent<BoxCollider2D>();
        else
            selectedObject->AddComponent<BoxCollider2D>();
        return;
    }

    const Rectangle rigidbodyButton = {panel.x + 15.0f, 690.0f, 170.0f, 25.0f};
    if (CheckCollisionPointRec(mousePosition, rigidbodyButton))
    {
        if (selectedObject->GetComponent<Rigidbody2D>() != nullptr)
            selectedObject->RemoveComponent<Rigidbody2D>();
        else
            selectedObject->AddComponent<Rigidbody2D>();
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

    DrawText(
        selectedObject->GetName().c_str(),
        static_cast<int>(panel.x + 15.0f),
        55,
        22,
        WHITE
    );

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
    rlPopMatrix();
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

    const BoxCollider2D* collider = object->GetComponent<BoxCollider2D>();
    const Rectangle colliderButton = {panel.x + 15.0f, 650.0f, 170.0f, 25.0f};
    DrawRectangleRec(colliderButton, collider == nullptr ? DARKGRAY : MAROON);
    DrawText(collider == nullptr ? "Add BoxCollider2D" : "Remove BoxCollider2D",
        static_cast<int>(colliderButton.x + 8.0f), static_cast<int>(colliderButton.y + 8.0f), 12, WHITE);
    if (collider != nullptr)
    {
        const Vector2 colliderSize = collider->GetSize();
        DrawText(TextFormat("Collider: %.0f x %.0f%s", colliderSize.x, colliderSize.y,
                collider->IsTrigger() ? " (Trigger)" : ""),
            static_cast<int>(panel.x + 195.0f), 657, 11, LIGHTGRAY);
    }
    const Rigidbody2D* body = object->GetComponent<Rigidbody2D>();
    const Rectangle rigidbodyButton = {panel.x + 15.0f, 690.0f, 170.0f, 25.0f};
    DrawRectangleRec(rigidbodyButton, body == nullptr ? DARKGRAY : MAROON);
    DrawText(body == nullptr ? "Add Rigidbody2D" : "Remove Rigidbody2D",
        static_cast<int>(rigidbodyButton.x + 8.0f), static_cast<int>(rigidbodyButton.y + 6.0f), 12, WHITE);
    if (body != nullptr)
        DrawText(TextFormat("Mass %.2f", body->GetMass()), static_cast<int>(panel.x + 195.0f), 697, 11, LIGHTGRAY);
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
    int key = InputManager::GetInstance().GetCharPressed();

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
