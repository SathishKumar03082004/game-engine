#pragma once
#include "raylib.h"
#include <string>
#include <vector>

class TopMenuBar
{
public:
    TopMenuBar();
    enum class Action { None, NewScene, SaveScene, BuildSettings, Undo, Redo, CreateObject,
        AddCollider, AddRigidbody, ToggleTilemap, ToggleUIEditor, ToggleHierarchy,
        ToggleInspector, ToggleProjectSettings, ToggleRenderSettings, ToggleAssetBrowser,
        ToggleAnimationWindow, ToggleAnimatorWindow,
        ToggleDebug, CreateCanvas, CreateEventSystem, CreateUIImage, CreateUIText,
        CreateUIButton, CreateUIInput, CreateUIPanel, CreateUILayout, About, Quit };
    Action Update();
    void Draw() const;
    bool BlocksMouse() const;
    bool WantsExit() const;
    void SetStatus(const std::string& value) { status = value; }
private:
    struct Item { const char* label; Action action; };
    struct Menu { const char* title; float x; float width; std::vector<Item> items; };
    std::vector<Menu> menus;
    int activeMenu = -1;
    bool wantsExit = false;
    bool consumedClick = false;
    std::string status;
};
