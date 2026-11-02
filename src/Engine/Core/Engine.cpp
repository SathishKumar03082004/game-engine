
#include "Engine.h"

#include <raylib.h>

#include "../Rendering/TextureManager.h"
#include "../../Editor/Inspector/RenderSettingsPanel.h"

// --------------------------------------------------
// Constructor
// --------------------------------------------------

Engine::Engine()
    : hierarchy(nullptr),
      inspector(nullptr),
      renderSettingsPanel(nullptr),
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
    TextureManager::GetInstance().Initialize();

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
            object->SetSprite(
                "resources/sprites/player.png"
            );

            object->SetRenderLayer(RenderLayer::Characters);
            object->SetSortingOrder(0);
        }
        else if (object->GetName() == "Enemy")
        {
            object->SetSprite(
                "resources/sprites/enemy.png"
            );

            object->SetRenderLayer(RenderLayer::Characters);
            object->SetSortingOrder(1);
        }
        else if (object->GetName() == "Platform")
        {
            object->SetSprite(
                "resources/sprites/platform.png"
            );

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

        bool allowInteraction =
            !overHierarchy &&
            !overInspector &&
            !overRenderSettings;

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

        EndDrawing();
    }
}

void Engine::Shutdown()
{
    delete renderSettingsPanel;
    renderSettingsPanel = nullptr;

    delete inspector;
    inspector = nullptr;

    delete hierarchy;
    hierarchy = nullptr;

    TextureManager::GetInstance().Shutdown();

    CloseWindow();
}
