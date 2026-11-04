#include "InputManager.h"

#include "Engine.h"

#include <raylib.h>

#include "../Resources/ResourceManager.h"
#include "../Resources/AssetReferences.h"
#include "../Animation/AnimationController.h"
#include "../Audio/AudioSystem.h"
#include "../Audio/AudioSource.h"
#include "../Audio/AudioListener.h"
#include "../../Editor/Inspector/RenderSettingsPanel.h"

// --------------------------------------------------
// Constructor
// --------------------------------------------------

Engine::Engine()
    : hierarchy(nullptr),
      inspector(nullptr),
      renderSettingsPanel(nullptr),
      assetBrowser(nullptr),
      projectSettingsPanel(nullptr),
      sceneDragObject(nullptr),
      sceneDragTransforms{},
      sceneDragging(false),
      gizmoDragObject(nullptr),
      gizmoDragTransforms{},
      gizmoWasDragging(false)
{
}

// --------------------------------------------------
// Initialize
// --------------------------------------------------

void Engine::Initialize()
{
    projectSettings.Load("resources/project.settings");
    SetConfigFlags(projectSettings.windowResizable ? FLAG_WINDOW_RESIZABLE : 0);
    InitWindow(
        projectSettings.windowWidth,
        projectSettings.windowHeight,
        projectSettings.projectName.c_str()
    );

    SetTargetFPS(projectSettings.targetFPS);

    // Initialize the shared texture manager.
    ResourceManager& resources = ResourceManager::GetInstance();
    resources.Initialize();
    resources.RegisterAsset(AssetReferences::PlayerSprite);
    resources.RegisterAsset(AssetReferences::EnemySprite);
    resources.RegisterAsset(AssetReferences::PlatformSprite);
    resources.RegisterAsset(AssetReferences::PlayerAnimationSheet);
    AudioSystem::GetInstance().Initialize();

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

    sceneManager.SwitchScene(scene, projectSettings.startupScene);
    for (const auto& object : scene.GetGameObjects())
    {
        if (object != nullptr && object->GetName() == "Player" && object->GetComponent<AudioListener>() == nullptr)
            object->AddComponent<AudioListener>();
    }

    InputManager::GetInstance().BindAction("MoveLeft", KEY_A);
    InputManager::GetInstance().BindAction("MoveRight", KEY_D);
    InputManager::GetInstance().BindAction("MoveUp", KEY_W);
    InputManager::GetInstance().BindAction("MoveDown", KEY_S);
    InputManager::GetInstance().BindAction("Jump", KEY_SPACE);

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

    projectSettingsPanel = new ProjectSettingsPanel(projectSettings, sceneManager, scene, history);
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
        InputManager::GetInstance().Update();
        debugOverlay.Update();
        // --------------------------------------------------
        // Update camera and scene
        // --------------------------------------------------

        camera.Update();

        const double sceneUpdateStart = GetTime();
        scene.Update();
        const float sceneUpdateMilliseconds = static_cast<float>((GetTime() - sceneUpdateStart) * 1000.0);

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
        if (projectSettingsPanel != nullptr) projectSettingsPanel->Update();

        // --------------------------------------------------
        // Mouse positions
        // --------------------------------------------------

        Vector2 mouseScreen = InputManager::GetInstance().GetMousePosition();

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
        bool overProjectSettings = projectSettingsPanel != nullptr && projectSettingsPanel->IsMouseOver();

        bool allowInteraction =
            !overHierarchy &&
            !overInspector &&
            !overRenderSettings &&
            !overAssetBrowser &&
            !overProjectSettings;

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
            gizmoDragTransforms.clear();
            for (GameObject* object : scene.GetSelectedObjects())
                if (object != nullptr) gizmoDragTransforms.push_back({object, CaptureTransform(object)});

            gizmoWasDragging = true;
        }

        // --------------------------------------------------
        // Record completed gizmo transform
        // --------------------------------------------------

        if (wasGizmoDragging && !isGizmoDragging)
        {
            if (gizmoWasDragging && gizmoDragObject != nullptr)
            {
                std::vector<TransformHistoryEntry> changes;
                for (const TransformSnapshot& snapshot : gizmoDragTransforms)
                    changes.push_back({snapshot.object, snapshot.transform, CaptureTransform(snapshot.object)});
                history.RecordTransforms(changes);
            }

            gizmoDragObject = nullptr;
            gizmoDragTransforms.clear();
            gizmoWasDragging = false;
        }

        // --------------------------------------------------
        // Select Object
        // --------------------------------------------------

        if (InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            allowInteraction &&
            !gizmoHandled)
        {
            const bool ctrlDown = InputManager::GetInstance().IsKeyDown(KEY_LEFT_CONTROL) || InputManager::GetInstance().IsKeyDown(KEY_RIGHT_CONTROL);
            GameObject* clickedObject = scene.GetObjectAt(mouseWorld);
            const bool wasSelected = scene.IsObjectSelected(clickedObject);
            scene.SelectObject(mouseWorld, ctrlDown);

            sceneDragObject =
                scene.GetSelectedObject();
            sceneDragging = clickedObject != nullptr && scene.IsObjectSelected(clickedObject) &&
                            !(ctrlDown && wasSelected);
            sceneDragTransforms.clear();
            if (sceneDragging)
            {
                scene.BeginSelectionDrag(mouseWorld);
                for (GameObject* object : scene.GetSelectedObjects())
                    if (object != nullptr) sceneDragTransforms.push_back({object, CaptureTransform(object)});
            }
        }

        // --------------------------------------------------
        // Drag Object
        // --------------------------------------------------

        if (InputManager::GetInstance().IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
            allowInteraction &&
            !gizmoHandled &&
            sceneDragging)
        {
            scene.DragSelection(mouseWorld);
        }

        // --------------------------------------------------
        // Record Completed Object Drag
        // --------------------------------------------------

        if (InputManager::GetInstance().IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            if (sceneDragging && sceneDragObject != nullptr)
            {
                std::vector<TransformHistoryEntry> changes;
                for (const TransformSnapshot& snapshot : sceneDragTransforms)
                    changes.push_back({snapshot.object, snapshot.transform, CaptureTransform(snapshot.object)});
                history.RecordTransforms(changes);
            }

            sceneDragObject = nullptr;
            sceneDragTransforms.clear();
            sceneDragging = false;
        }

        // --------------------------------------------------
        // Undo / Redo
        // --------------------------------------------------

        bool ctrlDown =
            InputManager::GetInstance().IsKeyDown(KEY_LEFT_CONTROL) ||
            InputManager::GetInstance().IsKeyDown(KEY_RIGHT_CONTROL);

        if (ctrlDown && InputManager::GetInstance().IsKeyPressed(KEY_Z))
        {
            history.Undo();
        }
        else if (ctrlDown && InputManager::GetInstance().IsKeyPressed(KEY_Y))
        {
            history.Redo();
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        camera.Begin();

        scene.Draw();
        debugOverlay.DrawWorld(scene);

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

        if (projectSettingsPanel != nullptr) projectSettingsPanel->Draw();
        debugOverlay.DrawScreen(scene, sceneUpdateMilliseconds);

        EndDrawing();
    }
}

void Engine::Shutdown()
{
    delete projectSettingsPanel;
    projectSettingsPanel = nullptr;
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
            if (AudioSource* source = object->GetComponent<AudioSource>()) source->Unload();
            AnimationComponent* animation = object->GetComponent<AnimationComponent>();
            if (animation != nullptr) animation->Unload();
        }
    }

    AudioSystem::GetInstance().Shutdown();
    ResourceManager::GetInstance().Shutdown();

    CloseWindow();
}
