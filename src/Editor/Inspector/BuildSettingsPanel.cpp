#include "BuildSettingsPanel.h"
#include "../FileDialog.h"
#include "../../Engine/Core/InputManager.h"
#include "../../Engine/Project/BuildConfiguration.h"
#include "../../Engine/Project/SceneManager.h"
#include "../../Engine/Scene/Scene.h"
#include <algorithm>

BuildSettingsPanel::BuildSettingsPanel(ProjectSettings& s, SceneManager& m, Scene& sc)
    : settings(s), scenes(m), scene(sc), panel{620,60,350,230} {}
bool BuildSettingsPanel::IsMouseOver() const
{ return open && CheckCollisionPointRec(InputManager::GetInstance().GetMousePosition(), panel); }
void BuildSettingsPanel::Update()
{
    if (!open || !InputManager::GetInstance().IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || !IsMouseOver()) return;
    const Vector2 mouse=InputManager::GetInstance().GetMousePosition();
    if (CheckCollisionPointRec(mouse,{panel.x+panel.width-28,panel.y+7,20,20})) { open=false; return; }
    if (CheckCollisionPointRec(mouse,{panel.x+15,panel.y+48,panel.width-30,34}))
    {
        static const char* targets[]={"release_x64","release_x86","release_arm64"};
        auto it=std::find_if(std::begin(targets),std::end(targets),[this](const char* value){return settings.buildConfiguration==value;});
        settings.buildConfiguration=targets[it==std::end(targets)?0:(it-std::begin(targets)+1)%3];
    }
    else if (CheckCollisionPointRec(mouse,{panel.x+15,panel.y+92,panel.width-30,34}))
    {
        const std::string folder = FileDialog::SelectFolder("Choose Standalone Build Output Folder",
            settings.outputDirectory, GetWindowHandle());
        if (!folder.empty()) settings.outputDirectory = folder;
    }
    else if (CheckCollisionPointRec(mouse,{panel.x+15,panel.y+142,145,34}))
        status=settings.Save("resources/project.settings")?"Build settings saved":"Could not save project settings";
    else if (CheckCollisionPointRec(mouse,{panel.x+170,panel.y+142,165,34}))
    {
        if (!scenes.GetActiveScenePath().empty()) settings.startupScene=scenes.GetActiveScenePath();
        status=settings.Save("resources/project.settings") && BuildConfiguration::GenerateWindowsBuildScript(settings)
            ? "Generated build_standalone.bat" : "Build script generation failed";
    }
}
void BuildSettingsPanel::Draw() const
{
    if (!open) return;
    DrawRectangleRec(panel,Color{35,35,35,250}); DrawRectangleLinesEx(panel,1,WHITE);
    DrawText("BUILD SETTINGS",(int)panel.x+14,(int)panel.y+12,18,WHITE);
    DrawRectangleRec({panel.x+panel.width-28,panel.y+7,20,20},MAROON); DrawText("x",(int)panel.x+(int)panel.width-22,(int)panel.y+10,14,WHITE);
    DrawText("Target platform",(int)panel.x+18,(int)panel.y+39,12,LIGHTGRAY);
    DrawRectangleRec({panel.x+15,panel.y+48,panel.width-30,34},DARKGRAY);
    DrawText(TextFormat("%s  (click to change)",settings.buildConfiguration.c_str()),(int)panel.x+25,(int)panel.y+59,14,YELLOW);
    DrawText("Output directory  (click to choose)",(int)panel.x+18,(int)panel.y+84,12,LIGHTGRAY);
    DrawRectangleRec({panel.x+15,panel.y+92,panel.width-30,34},DARKGRAY);
    DrawText(TextFormat("%s  (click to change)",settings.outputDirectory.c_str()),(int)panel.x+25,(int)panel.y+103,13,WHITE);
    DrawRectangleRec({panel.x+15,panel.y+142,145,34},DARKGREEN); DrawText("Save Settings",(int)panel.x+45,(int)panel.y+153,13,WHITE);
    DrawRectangleRec({panel.x+170,panel.y+142,165,34},DARKBLUE); DrawText("Generate Build Script",(int)panel.x+182,(int)panel.y+153,11,WHITE);
    DrawText(status.c_str(),(int)panel.x+15,(int)panel.y+195,11,GREEN);
}
