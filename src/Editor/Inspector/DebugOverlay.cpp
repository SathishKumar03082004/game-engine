#include "DebugOverlay.h"
#include "../../Engine/Core/InputManager.h"
#include "../../Engine/Physics/BoxCollider2D.h"

void DebugOverlay::Update()
{
    InputManager& input = InputManager::GetInstance();
    if (input.IsKeyPressed(KEY_F3)) visible = !visible;
    if (input.IsKeyPressed(KEY_F4)) showColliders = !showColliders;
}

void DebugOverlay::DrawWorld(const Scene& scene) const
{
    if (!showColliders) return;
    for (const auto& object : scene.GetGameObjects())
    {
        if (object == nullptr) continue;
        for (const auto& component : object->GetComponents())
        {
            const auto* collider = dynamic_cast<const BoxCollider2D*>(component.get());
            if (collider == nullptr || !collider->IsEnabled()) continue;
            DrawRectangleLinesEx(collider->GetWorldBounds(), 2.0f,
                collider->IsTrigger() ? ORANGE : LIME);
        }
    }
    for (const SolidTileBounds& tile : scene.GetTilemap().GetSolidTileBounds())
        DrawRectangleLinesEx(tile.bounds, 1.0f, ORANGE);
}

void DebugOverlay::DrawScreen(const Scene& scene, float sceneUpdateMilliseconds) const
{
    if (!visible) return;
    const int collisions = static_cast<int>(scene.GetPhysicsSystem().GetCollisionPairs().size());
    const int objects = static_cast<int>(scene.GetGameObjects().size());
    const Rectangle panel{260.0f, 235.0f, 205.0f, 92.0f};
    DrawRectangleRec(panel, Color{20, 20, 20, 220});
    DrawRectangleLinesEx(panel, 1.0f, SKYBLUE);
    DrawText(TextFormat("FPS: %d  Frame: %.2f ms", GetFPS(), GetFrameTime() * 1000.0f), 270, 244, 12, WHITE);
    DrawText(TextFormat("Scene update: %.2f ms", sceneUpdateMilliseconds), 270, 263, 12, LIGHTGRAY);
    DrawText(TextFormat("Objects: %d  Contacts: %d", objects, collisions), 270, 282, 12, LIGHTGRAY);
    DrawText(showColliders ? "F4: colliders ON | F3: hide" : "F4: colliders | F3: hide", 270, 304, 10, YELLOW);
}

bool DebugOverlay::IsVisible() const { return visible; }
