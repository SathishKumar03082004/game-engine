
#include "Engine.h"

#include <raylib.h>

#include "../Resources/ResourceManager.h"
#include "../Resources/AssetReferences.h"
#include "../Animation/AnimationController.h"
#include "../../Editor/Inspector/RenderSettingsPanel.h"

// --------------------------------------------------
// Constructor
// --------------------------------------------------

Engine::Engine()
    : hierarchy(nullptr),
      inspector(nullptr),
      renderSettingsPanel(nullptr),
      assetBrowser(nullptr),
      sceneDragObject(nullptr),
      sceneDragStart{},
      sceneDragging(false),
      gizmoDragObject(nullptr),
      gizmoDragStart{},
      gizmoWasDragging(false)
{
}

// --------------------------------------------------
// Initialize
// --------------------------------------------------

void Engine::Initialize()
{
    InitWindow(
        1280,
        720,
        "My 2D Engine"
    );

    SetTargetFPS(60);

    // Initialize the shared texture manager.
    ResourceManager& resources = ResourceManager::GetInstance();
    resources.Initialize();
    resources.RegisterAsset(AssetReferences::PlayerSprite);
    resources.RegisterAsset(AssetReferences::EnemySprite);
    resources.RegisterAsset(AssetReferences::PlatformSprite);
    resources.RegisterAsset(AssetReferences::PlayerAnimationSheet);

    camera.Initialize();

    scene.Initialize();

    // --------------------------------------------------
    // Load initial sprites
    // --------------------------------------------------

    auto& objects = scene.GetGameObjects();

    for (const auto& object : objects)
    {
        if (!object)
        {
            continue;
        }

        if (object->GetName() == "Player")
        {
            object->SetSprite(AssetReferences::PlayerSprite);

            AnimationComponent& animation = object->GetAnimation();
            animation.AddAnimation("Idle", AssetReferences::PlayerAnimationSheet, 44, 39, 1, 1.0f, true);
            animation.AddAnimation("Walk", AssetReferences::PlayerAnimationSheet, 44, 39, 6, 8.0f, true);
            animation.AddAnimation("Run", AssetReferences::PlayerAnimationSheet, 44, 39, 6, 12.0f, true);
            animation.AddAnimation("Jump", AssetReferences::PlayerAnimationSheet, 44, 39, 6, 10.0f, false);

            AnimationController& controller = object->AddComponent<AnimationController>();
            controller.SetDefaultState("Idle");
            controller.AddBoolTransition("Idle", "Walk", "moving", true);
            controller.AddBoolTransition("Walk", "Idle", "moving", false);
            controller.AddBoolTransition("Walk", "Run", "running", true);
            controller.AddBoolTransition("Run", "Walk", "running", false);
            controller.AddTriggerTransition("Idle", "Jump", "jump");
            controller.AddTriggerTransition("Walk", "Jump", "jump");
            controller.AddTriggerTransition("Run", "Jump", "jump");
            controller.AddCompletionTransition("Jump", "Idle");

            object->SetRenderLayer(RenderLayer::Characters);
            object->SetSortingOrder(0);
        }
        else if (object->GetName() == "Enemy")
        {
            object->SetSprite(AssetReferences::EnemySprite);

            object->SetRenderLayer(RenderLayer::Characters);
            object->SetSortingOrder(1);
        }
        else if (object->GetName() == "Platform")
        {
            object->SetSprite(AssetReferences::PlatformSprite);

            object->SetRenderLayer(RenderLayer::Default);
            object->SetSortingOrder(0);
        }
    }

    // --------------------------------------------------
    // Initialize editor panels
    // --------------------------------------------------

    hierarchy = new Hierarchy(scene, history);
    hierarchy->Initialize();

    inspector = new Inspector(scene);
    inspector->Initialize();

    renderSettingsPanel = new RenderSettingsPanel(scene);
    renderSettingsPanel->Initialize();

    assetBrowser = new AssetBrowser();
    assetBrowser->Initialize();
}

// --------------------------------------------------
// Capture Transform
// --------------------------------------------------

TransformState Engine::CaptureTransform(GameObject* object)
{
    if (object == nullptr)
    {
        return {};
    }

    TransformComponent& transform =
        object->GetTransform();

    return
    {
        transform.GetPosition(),
        transform.GetRotation(),
        transform.GetScale()
    };
}

// --------------------------------------------------
// Main Loop
// --------------------------------------------------

void Engine::Run()
{
    while (!WindowShouldClose())
    {
        // --------------------------------------------------
        // Update camera and scene
        // --------------------------------------------------

        camera.Update();

        scene.Update();

        // --------------------------------------------------
        // Update editor panels
        // --------------------------------------------------

        if (hierarchy != nullptr)
        {
            hierarchy->Update();
        }

        if (inspector != nullptr)
        {
            inspector->Update();
        }

        if (renderSettingsPanel != nullptr)
        {
            renderSettingsPanel->Update();
        }

        if (assetBrowser != nullptr)
        {
            assetBrowser->Update();
        }

        // --------------------------------------------------
        // Mouse positions
        // --------------------------------------------------

        Vector2 mouseScreen = GetMousePosition();

        Camera2D currentCamera =
            camera.GetCamera();

        Vector2 mouseWorld =
            GetScreenToWorld2D(
                mouseScreen,
                currentCamera
            );

        // --------------------------------------------------
        // Prevent scene interactions over editor panels
        // --------------------------------------------------

        bool overHierarchy =
            hierarchy != nullptr &&
            hierarchy->IsMouseOver();

        bool overInspector =
            inspector != nullptr &&
            inspector->IsMouseOver();

        bool overRenderSettings =
            renderSettingsPanel != nullptr &&
            renderSettingsPanel->IsMouseOver();

        bool overAssetBrowser =
            assetBrowser != nullptr &&
            assetBrowser->IsMouseOver();

        bool allowInteraction =
            !overHierarchy &&
            !overInspector &&
            !overRenderSettings &&
            !overAssetBrowser;

        if (assetBrowser != nullptr)
        {
            const std::string droppedAsset = assetBrowser->ConsumeDroppedAssetPath();
            if (!droppedAsset.empty() && allowInteraction)
            {
                GameObject* target = scene.GetObjectAt(mouseWorld);
                if (target != nullptr && target->SetSprite(droppedAsset))
                {
                    AnimationComponent* animation = target->GetComponent<AnimationComponent>();
                    if (animation != nullptr) animation->SetEnabled(false);
                    AnimationController* controller = target->GetComponent<AnimationController>();
                    if (controller != nullptr) controller->SetEnabled(false);
                }
            }
        }

        // --------------------------------------------------
        // Transform Gizmo
        // --------------------------------------------------

        bool wasGizmoDragging =
            transformGizmo.IsDragging();

        bool gizmoHandled =
            transformGizmo.Update(
                scene,
                mouseWorld,
                currentCamera.zoom,
                allowInteraction
            );

        bool isGizmoDragging =
            transformGizmo.IsDragging();

        // --------------------------------------------------
        // Start recording gizmo transform
        // --------------------------------------------------

        if (!wasGizmoDragging && isGizmoDragging)
        {
            gizmoDragObject =
                scene.GetSelectedObject();

            gizmoDragStart =
                CaptureTransform(gizmoDragObject);

            gizmoWasDragging = true;
        }

        // --------------------------------------------------
        // Record completed gizmo transform
        // --------------------------------------------------

        if (wasGizmoDragging && !isGizmoDragging)
        {
            if (gizmoWasDragging &&
                gizmoDragObject != nullptr)
            {
                TransformState after =
                    CaptureTransform(gizmoDragObject);

                history.RecordTransform(
                    gizmoDragObject,
                    gizmoDragStart,
                    after
                );
            }

            gizmoDragObject = nullptr;
            gizmoWasDragging = false;
        }

        // --------------------------------------------------
        // Select Object
        // --------------------------------------------------

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            allowInteraction &&
            !gizmoHandled)
        {
            scene.SelectObject(mouseWorld);

            sceneDragObject =
                scene.GetSelectedObject();

            sceneDragStart =
                CaptureTransform(sceneDragObject);

            sceneDragging =
                sceneDragObject != nullptr;
        }

        // --------------------------------------------------
        // Drag Object
        // --------------------------------------------------

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
            allowInteraction &&
            !gizmoHandled)
        {
            scene.DragSelectedObject(mouseWorld);
        }

        // --------------------------------------------------
        // Record Completed Object Drag
        // --------------------------------------------------

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            if (sceneDragging &&
                sceneDragObject != nullptr)
            {
                TransformState after =
                    CaptureTransform(sceneDragObject);

                history.RecordTransform(
                    sceneDragObject,
                    sceneDragStart,
                    after
                );
            }

            sceneDragObject = nullptr;
            sceneDragging = false;
        }

        // --------------------------------------------------
        // Undo / Redo
        // --------------------------------------------------

        bool ctrlDown =
            IsKeyDown(KEY_LEFT_CONTROL) ||
            IsKeyDown(KEY_RIGHT_CONTROL);

        if (ctrlDown && IsKeyPressed(KEY_Z))
        {
            history.Undo();
        }
        else if (ctrlDown && IsKeyPressed(KEY_Y))
        {
            history.Redo();
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        camera.Begin();

        scene.Draw();

        transformGizmo.Draw(
            scene,
            currentCamera.zoom
        );

        camera.End();

        if (hierarchy != nullptr)
        {
            hierarchy->Draw();
        }

        if (inspector != nullptr)
        {
            inspector->Draw();
        }

        if (renderSettingsPanel != nullptr)
        {
            renderSettingsPanel->Draw();
        }

        if (assetBrowser != nullptr)
        {
            assetBrowser->Draw();
        }

        EndDrawing();
    }
}

void Engine::Shutdown()
{
    if (assetBrowser != nullptr)
    {
        assetBrowser->Shutdown();
        delete assetBrowser;
        assetBrowser = nullptr;
    }

    delete renderSettingsPanel;
    renderSettingsPanel = nullptr;

    delete inspector;
    inspector = nullptr;

    delete hierarchy;
    hierarchy = nullptr;

    // Release component-held references while the graphics context is alive.
    for (const auto& object : scene.GetGameObjects())
    {
        if (object != nullptr)
        {
            AnimationComponent* animation = object->GetComponent<AnimationComponent>();
            if (animation != nullptr) animation->Unload();
        }
    }

    ResourceManager::GetInstance().Shutdown();

    CloseWindow();
}
