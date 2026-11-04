#include "ProjectSettingsPanel.h"
#include "../../Engine/Project/BuildConfiguration.h"

#include <algorithm>

ProjectSettingsPanel::ProjectSettingsPanel(ProjectSettings& projectSettings, SceneManager& sceneManager,
    Scene& activeScene, EditorHistory& editorHistory)
    : settings(projectSettings), scenes(sceneManager), scene(activeScene), history(editorHistory),
      panel{650.0f, 15.0f, 310.0f, 440.0f}
{
}

bool ProjectSettingsPanel::IsMouseOver() const
{
    return CheckCollisionPointRec(GetMousePosition(), panel);
}

void ProjectSettingsPanel::Update()
{
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || !IsMouseOver()) return;
    const Vector2 mouse = GetMousePosition();
    const Rectangle widthMinus{panel.x + 10.0f, panel.y + 40.0f, 32.0f, 26.0f};
    const Rectangle widthPlus{panel.x + 268.0f, panel.y + 40.0f, 32.0f, 26.0f};
    const Rectangle heightMinus{panel.x + 10.0f, panel.y + 75.0f, 32.0f, 26.0f};
    const Rectangle heightPlus{panel.x + 268.0f, panel.y + 75.0f, 32.0f, 26.0f};
    const Rectangle fpsMinus{panel.x + 10.0f, panel.y + 110.0f, 32.0f, 26.0f};
    const Rectangle fpsPlus{panel.x + 268.0f, panel.y + 110.0f, 32.0f, 26.0f};
    if (CheckCollisionPointRec(mouse, widthMinus)) settings.windowWidth = std::max(320, settings.windowWidth - 160);
    else if (CheckCollisionPointRec(mouse, widthPlus)) settings.windowWidth = std::min(7680, settings.windowWidth + 160);
    else if (CheckCollisionPointRec(mouse, heightMinus)) settings.windowHeight = std::max(240, settings.windowHeight - 90);
    else if (CheckCollisionPointRec(mouse, heightPlus)) settings.windowHeight = std::min(4320, settings.windowHeight + 90);
    else if (CheckCollisionPointRec(mouse, fpsMinus)) settings.targetFPS = std::max(15, settings.targetFPS - 15);
    else if (CheckCollisionPointRec(mouse, fpsPlus)) settings.targetFPS = std::min(360, settings.targetFPS + 15);
    else if (CheckCollisionPointRec(mouse, {panel.x + 10.0f, panel.y + 145.0f, 290.0f, 26.0f}))
        settings.windowResizable = !settings.windowResizable;
    else if (CheckCollisionPointRec(mouse, {panel.x + 10.0f, panel.y + 180.0f, 90.0f, 28.0f}))
    {
        if (scenes.SwitchRelative(scene, -1)) { history.Clear(); settings.startupScene = scenes.GetActiveScenePath(); }
    }
    else if (CheckCollisionPointRec(mouse, {panel.x + 110.0f, panel.y + 180.0f, 90.0f, 28.0f}))
    {
        if (scenes.SwitchRelative(scene, 1)) { history.Clear(); settings.startupScene = scenes.GetActiveScenePath(); }
    }
    else if (CheckCollisionPointRec(mouse, {panel.x + 210.0f, panel.y + 180.0f, 90.0f, 28.0f}))
    {
        if (scenes.CreateScene(scene, "Scene")) { history.Clear(); settings.startupScene = scenes.GetActiveScenePath(); status = "Created scene"; }
        else status = "Could not create scene";
    }
    else if (CheckCollisionPointRec(mouse, {panel.x + 10.0f, panel.y + 240.0f, 290.0f, 28.0f}))
        status = scenes.SaveActiveScene(scene) ? "Scene saved" : "Scene save failed";
    else if (CheckCollisionPointRec(mouse, {panel.x + 10.0f, panel.y + 280.0f, 290.0f, 28.0f}))
    {
        static const char* configs[] = {"release_x64", "release_x86", "release_arm64"};
        const auto current = std::find_if(std::begin(configs), std::end(configs), [this](const char* value)
            { return settings.buildConfiguration == value; });
        const std::size_t next = current == std::end(configs) ? 0 :
            (static_cast<std::size_t>(current - std::begin(configs)) + 1) % std::size(configs);
        settings.buildConfiguration = configs[next];
    }
    else if (CheckCollisionPointRec(mouse, {panel.x + 10.0f, panel.y + 310.0f, 290.0f, 18.0f}))
        settings.outputDirectory = settings.outputDirectory == "build/standalone" ? "dist" : "build/standalone";
    else if (CheckCollisionPointRec(mouse, {panel.x + 10.0f, panel.y + 330.0f, 140.0f, 30.0f}))
    {
        settings.startupScene = scenes.GetActiveScenePath().empty() ? settings.startupScene : scenes.GetActiveScenePath();
        status = settings.Save("resources/project.settings") ? "Project settings saved" : "Settings save failed";
    }
    else if (CheckCollisionPointRec(mouse, {panel.x + 160.0f, panel.y + 330.0f, 140.0f, 30.0f}))
    {
        if (!scenes.GetActiveScenePath().empty()) settings.startupScene = scenes.GetActiveScenePath();
        const bool settingsSaved = settings.Save("resources/project.settings");
        status = settingsSaved && BuildConfiguration::GenerateWindowsBuildScript(settings)
            ? "Generated build_standalone.bat" : "Build config generation failed";
    }

    if (CheckCollisionPointRec(mouse, widthMinus) || CheckCollisionPointRec(mouse, widthPlus) ||
        CheckCollisionPointRec(mouse, heightMinus) || CheckCollisionPointRec(mouse, heightPlus) ||
        CheckCollisionPointRec(mouse, fpsMinus) || CheckCollisionPointRec(mouse, fpsPlus) ||
        CheckCollisionPointRec(mouse, {panel.x + 10.0f, panel.y + 145.0f, 290.0f, 26.0f}))
    {
        SetTargetFPS(settings.targetFPS);
        SetWindowSize(settings.windowWidth, settings.windowHeight);
        status = "Settings changed; save to persist. Restart to apply window flags.";
    }
}

void ProjectSettingsPanel::Draw() const
{
    DrawRectangleRec(panel, Color{35, 35, 35, 245});
    DrawRectangleLinesEx(panel, 1.0f, WHITE);
    DrawText("PROJECT SETTINGS", static_cast<int>(panel.x + 12.0f), static_cast<int>(panel.y + 10.0f), 16, WHITE);
    auto valueRow = [this](const char* label, const std::string& value, float y)
    {
        DrawText(label, static_cast<int>(panel.x + 48.0f), static_cast<int>(panel.y + y + 6.0f), 12, LIGHTGRAY);
        DrawText(value.c_str(), static_cast<int>(panel.x + 125.0f), static_cast<int>(panel.y + y + 6.0f), 13, YELLOW);
        DrawRectangleRec({panel.x + 10.0f, panel.y + y, 32.0f, 26.0f}, DARKGRAY);
        DrawRectangleRec({panel.x + 268.0f, panel.y + y, 32.0f, 26.0f}, DARKGRAY);
        DrawText("-", static_cast<int>(panel.x + 22.0f), static_cast<int>(panel.y + y + 5.0f), 16, WHITE);
        DrawText("+", static_cast<int>(panel.x + 280.0f), static_cast<int>(panel.y + y + 5.0f), 16, WHITE);
    };
    valueRow("Width", std::to_string(settings.windowWidth), 40.0f);
    valueRow("Height", std::to_string(settings.windowHeight), 75.0f);
    valueRow("Target FPS", std::to_string(settings.targetFPS), 110.0f);
    DrawRectangleRec({panel.x + 10.0f, panel.y + 145.0f, 290.0f, 26.0f}, Color{55, 55, 55, 255});
    DrawText(settings.windowResizable ? "Resizable window: ON" : "Resizable window: OFF",
        static_cast<int>(panel.x + 14.0f), static_cast<int>(panel.y + 150.0f), 13, WHITE);
    const char* labels[] = {"Previous", "Next", "New Scene"};
    const float xs[] = {10.0f, 110.0f, 210.0f};
    for (int i = 0; i < 3; ++i)
    {
        Rectangle button{panel.x + xs[i], panel.y + 180.0f, 90.0f, 28.0f};
        DrawRectangleRec(button, DARKGRAY);
        DrawText(labels[i], static_cast<int>(button.x + 8.0f), static_cast<int>(button.y + 8.0f), 11, WHITE);
    }
    const std::string active = scenes.GetActiveScenePath().empty() ? "No active scene" : scenes.GetActiveScenePath();
    DrawText(TextFormat("Active: %.42s", active.c_str()), static_cast<int>(panel.x + 10.0f), static_cast<int>(panel.y + 218.0f), 10, LIGHTGRAY);
    DrawRectangleRec({panel.x + 10.0f, panel.y + 240.0f, 290.0f, 28.0f}, DARKGRAY);
    DrawText("Save Active Scene", static_cast<int>(panel.x + 90.0f), static_cast<int>(panel.y + 248.0f), 12, WHITE);
    DrawRectangleRec({panel.x + 10.0f, panel.y + 280.0f, 290.0f, 28.0f}, DARKGRAY);
    DrawText(TextFormat("Build target: %.22s", settings.buildConfiguration.c_str()),
        static_cast<int>(panel.x + 30.0f), static_cast<int>(panel.y + 288.0f), 12, WHITE);
    DrawText(TextFormat("Output (click to change): %.28s", settings.outputDirectory.c_str()),
        static_cast<int>(panel.x + 10.0f), static_cast<int>(panel.y + 318.0f), 10, LIGHTGRAY);
    DrawRectangleRec({panel.x + 10.0f, panel.y + 330.0f, 140.0f, 30.0f}, DARKGREEN);
    DrawText("Save Settings", static_cast<int>(panel.x + 24.0f), static_cast<int>(panel.y + 339.0f), 11, WHITE);
    DrawRectangleRec({panel.x + 160.0f, panel.y + 330.0f, 140.0f, 30.0f}, DARKBLUE);
    DrawText("Generate Build", static_cast<int>(panel.x + 174.0f), static_cast<int>(panel.y + 339.0f), 11, WHITE);
    DrawText(status.c_str(), static_cast<int>(panel.x + 10.0f), static_cast<int>(panel.y + 380.0f), 9, GREEN);
}
