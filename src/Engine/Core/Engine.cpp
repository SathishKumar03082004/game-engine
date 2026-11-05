#include "InputManager.h"

#include "Engine.h"

#include <raylib.h>
#include <raymath.h>
#include <algorithm>
#include <cmath>

#include "../Resources/ResourceManager.h"
#include "../Resources/AssetReferences.h"
#include "../Animation/AnimationController.h"
#include "../Audio/AudioSystem.h"
#include "../Audio/AudioSource.h"
#include "../Audio/AudioListener.h"
#include "../Rendering/PointLight2D.h"
#include "../Rendering/ParticleEmitter.h"
#include "../Physics/BoxCollider2D.h"
#include "../UI/UIComponent.h"
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
      tilemapEditor(nullptr),
      uiEditor(nullptr),
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

    sceneTarget = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    compositeShader = LoadShaderFromMemory(nullptr,
        "#version 330\nin vec2 fragTexCoord; in vec4 fragColor; out vec4 finalColor; uniform sampler2D texture0; uniform int effectMode; void main(){ vec4 c=texture(texture0,fragTexCoord)*fragColor; if(effectMode==1){float g=dot(c.rgb,vec3(0.299,0.587,0.114)); c.rgb=vec3(g);} else if(effectMode==2){c.rgb=vec3(c.r*1.08,c.g*0.98,c.b*0.82);} finalColor=c; }");
    effectLocation = GetShaderLocation(compositeShader, "effectMode");

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
    bool hasCanvas = false;
    for (const auto& object : scene.GetGameObjects()) if (object && object->GetComponent<UICanvas>()) hasCanvas = true;
    if (!hasCanvas)
    {
        GameObject* canvas = scene.CreateGameObject("Canvas");
        if (canvas != nullptr)
        {
            canvas->AddComponent<UICanvas>();
            GameObject* titleObject = scene.CreateGameObject("Main Menu Title");
            titleObject->SetParent(canvas);
            UITextComponent& title = titleObject->AddComponent<UITextComponent>();
            title.text = "MY 2D GAME"; title.fontSize = 42; title.anchor = {0.5f, 0.28f}; title.size = {420, 56};
            GameObject* buttonObject = scene.CreateGameObject("Start Button");
            buttonObject->SetParent(canvas);
            UIButtonComponent& startButton = buttonObject->AddComponent<UIButtonComponent>();
            startButton.label = "Start Game"; startButton.anchor = {0.5f, 0.48f}; startButton.size = {220, 52};
            GameObject* inputObject = scene.CreateGameObject("Player Name Field");
            inputObject->SetParent(canvas);
            UIInputField& playerName = inputObject->AddComponent<UIInputField>();
            playerName.placeholder = "Player name"; playerName.anchor = {0.5f, 0.59f}; playerName.size = {280, 44};
            startButton.onClick = [&title, &startButton, &playerName]() { title.SetEnabled(false); startButton.SetEnabled(false); playerName.SetEnabled(false); };
            GameObject* hudObject = scene.CreateGameObject("HUD");
            hudObject->SetParent(canvas);
            UITextComponent& hud = hudObject->AddComponent<UITextComponent>();
            hud.text = "HP 100 / 100"; hud.fontSize = 20; hud.anchor = {0.08f, 0.08f}; hud.size = {180, 28};
            GameObject* healthBarObject = scene.CreateGameObject("Health Bar");
            healthBarObject->SetParent(canvas);
            UIProgressBar& healthBar = healthBarObject->AddComponent<UIProgressBar>();
            healthBar.anchor = {0.12f, 0.13f}; healthBar.size = {190, 16};
        }
    }
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
    tilemapEditor = new TilemapEditor(scene, *assetBrowser);
    uiEditor = new UIEditor(scene);
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
        if (uiEditor != nullptr) uiEditor->Update();
        uiManager.Update(scene);

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

        if (tilemapEditor != nullptr) tilemapEditor->Update(mouseWorld);

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
        bool tilemapEditing = tilemapEditor != nullptr && tilemapEditor->IsActive();

        bool allowInteraction =
            !overHierarchy &&
            !overInspector &&
            !overRenderSettings &&
            !overAssetBrowser &&
            !overProjectSettings &&
            !tilemapEditing;

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

        if (InputManager::GetInstance().IsKeyPressed(KEY_F6)) screenEffect = screenEffect >= 2.0f ? 0.0f : screenEffect + 1.0f;
        if (InputManager::GetInstance().IsKeyPressed(KEY_F8))
        {
            GameObject* selected = scene.GetSelectedObject();
            if (selected != nullptr)
            {
                ParticleEmitter* emitter = selected->GetComponent<ParticleEmitter>();
                if (emitter == nullptr) emitter = &selected->AddComponent<ParticleEmitter>();
                else emitter->SetEnabled(!emitter->IsEnabled());
            }
        }
        if (GameObject* selected = scene.GetSelectedObject())
        {
            if (ParticleEmitter* emitter = selected->GetComponent<ParticleEmitter>())
            {
                if (InputManager::GetInstance().IsKeyPressed(KEY_F9)) emitter->emissionRate = std::max(0.0f, emitter->emissionRate + (IsKeyDown(KEY_LEFT_SHIFT) ? -2.0f : 2.0f));
                if (InputManager::GetInstance().IsKeyPressed(KEY_F10)) emitter->lifetime = std::max(0.1f, emitter->lifetime + (IsKeyDown(KEY_LEFT_SHIFT) ? -0.1f : 0.1f));
            }
        }

        if (!IsRenderTextureValid(sceneTarget) || sceneTarget.texture.width != GetScreenWidth() || sceneTarget.texture.height != GetScreenHeight())
        {
            if (IsRenderTextureValid(sceneTarget)) UnloadRenderTexture(sceneTarget);
            sceneTarget = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
        }
        BeginDrawing();
        ClearBackground(BLACK);
        BeginTextureMode(sceneTarget);
        ClearBackground(RAYWHITE);
        currentCamera = camera.GetCamera();
        parallax.Draw(currentCamera.target, GetScreenWidth(), GetScreenHeight());
        camera.Begin();

        currentCamera = camera.GetCamera();
        scene.Draw();
        // Ambient darkness and point lights are drawn in a multiplicative/additive lighting pass.
        const float viewW = GetScreenWidth() / currentCamera.zoom;
        const float viewH = GetScreenHeight() / currentCamera.zoom;
        BeginBlendMode(BLEND_MULTIPLIED);
        DrawRectangle(static_cast<int>(currentCamera.target.x - viewW * 0.5f), static_cast<int>(currentCamera.target.y - viewH * 0.5f),
            static_cast<int>(viewW), static_cast<int>(viewH), Color{78, 78, 88, 255});
        EndBlendMode();
        BeginBlendMode(BLEND_ADDITIVE);
        for (const auto& object : scene.GetGameObjects())
        {
            if (object == nullptr) continue;
            const PointLight2D* light = object->GetComponent<PointLight2D>();
            if (light == nullptr || !light->IsEnabled()) continue;
            BeginBlendMode(BLEND_MULTIPLIED);
            for (const auto& caster : scene.GetGameObjects())
            {
                if (caster == nullptr) continue;
                const BoxCollider2D* collider = caster->GetComponent<BoxCollider2D>();
                if (collider == nullptr || !collider->IsEnabled()) continue;
                const Rectangle b = collider->GetWorldBounds(); const Vector2 lp = object->GetWorldPosition();
                const Vector2 corners[4] = {{b.x,b.y},{b.x+b.width,b.y},{b.x+b.width,b.y+b.height},{b.x,b.y+b.height}};
                for (int c = 0; c < 4; ++c) { Vector2 a = corners[c], z = corners[(c+1)%4];
                    Vector2 da = Vector2Normalize(Vector2Subtract(a, lp)), dz = Vector2Normalize(Vector2Subtract(z, lp));
                    Vector2 ae = Vector2Add(a, Vector2Scale(da, light->radius * 3.0f)), ze = Vector2Add(z, Vector2Scale(dz, light->radius * 3.0f));
                    DrawTriangle(a, z, ze, Color{0,0,0,210}); DrawTriangle(a, ze, ae, Color{0,0,0,210});
                }
            }
            EndBlendMode();
            Color inner = light->color; inner.a = static_cast<unsigned char>(255.0f * std::clamp(light->intensity, 0.0f, 1.0f));
            Color outer = inner; outer.a = 0;
            DrawCircleGradient(object->GetWorldPosition(), light->radius, inner, outer);
        }
        EndBlendMode();
        debugOverlay.DrawWorld(scene);

        transformGizmo.Draw(
            scene,
            currentCamera.zoom
        );

        camera.End();
        EndTextureMode();
        int effect = static_cast<int>(screenEffect);
        if (effectLocation >= 0) SetShaderValue(compositeShader, effectLocation, &effect, SHADER_UNIFORM_INT);
        BeginShaderMode(compositeShader);
        DrawTexturePro(sceneTarget.texture, {0, 0, static_cast<float>(sceneTarget.texture.width), -static_cast<float>(sceneTarget.texture.height)},
            {0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())}, {0, 0}, 0, WHITE);
        EndShaderMode();

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
        if (tilemapEditor != nullptr) tilemapEditor->Draw();
        uiManager.Draw(scene);
        if (uiEditor != nullptr) uiEditor->Draw();
        debugOverlay.DrawScreen(scene, sceneUpdateMilliseconds);
        if (debugOverlay.IsVisible()) DrawText("F6: post effect | F8: toggle particle emitter on selection", 270, 328, 10, GOLD);

        EndDrawing();
    }
}

void Engine::Shutdown()
{
    if (IsRenderTextureValid(sceneTarget)) UnloadRenderTexture(sceneTarget);
    if (IsShaderValid(compositeShader)) UnloadShader(compositeShader);
    parallax.Clear();
    uiManager.ReleaseResources(scene);
    delete tilemapEditor;
    tilemapEditor = nullptr;
    delete uiEditor;
    uiEditor = nullptr;
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

ParallaxBackground& Engine::GetParallaxBackground() { return parallax; }
