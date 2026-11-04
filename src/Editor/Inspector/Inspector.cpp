#include "../../Engine/Core/InputManager.h"

#include "Inspector.h"
#include "../../Engine/Animation/AnimationController.h"
#include "../../Engine/Physics/BoxCollider2D.h"
#include "../../Engine/Physics/Rigidbody2D.h"

#include <cstdlib>
#include <string>

// --------------------------------------------------
// Constructor
// --------------------------------------------------

Inspector::Inspector(Scene& sceneReference)
    : scene(sceneReference)
{
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
    width = 300.0f;
    height = static_cast<float>(GetScreenHeight());

    panel.x = static_cast<float>(GetScreenWidth()) - width;
    panel.y = 0.0f;
    panel.width = width;
    panel.height = height;

    Vector2 mousePosition = InputManager::GetInstance().GetMousePosition();

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

        return;
    }

    DrawText(
        selectedObject->GetName().c_str(),
        static_cast<int>(panel.x + 15.0f),
        55,
        22,
        WHITE
    );

    if (animationTab)
    {
        DrawAnimationEditor(selectedObject);
        return;
    }

    DrawTransform(selectedObject);

    DrawSprite(selectedObject);
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
    return CheckCollisionPointRec(
        InputManager::GetInstance().GetMousePosition(),
        panel
    );
}
