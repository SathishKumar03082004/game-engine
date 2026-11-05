#include "TopMenuBar.h"
#include "../../Engine/Core/InputManager.h"
#include <cstring>

TopMenuBar::TopMenuBar()
{
    const std::vector<std::pair<const char*, std::vector<Item>>> definitions = {
        {"File", {{"New Scene",Action::NewScene},{"Save Scene",Action::SaveScene},{"Build Settings...",Action::BuildSettings},{"Quit",Action::Quit}}},
        {"Edit", {{"Undo",Action::Undo},{"Redo",Action::Redo}}},
        {"Assets", {{"Asset Browser",Action::ToggleAssetBrowser}}},
        {"GameObject", {{"Create Empty",Action::CreateObject},{"Canvas",Action::CreateCanvas},{"Event System",Action::CreateEventSystem},{"UI Image",Action::CreateUIImage},{"UI Text",Action::CreateUIText},{"UI Button",Action::CreateUIButton},{"UI Input Field",Action::CreateUIInput},{"UI Panel",Action::CreateUIPanel},{"UI Layout Group",Action::CreateUILayout}}},
        {"Component", {{"Add BoxCollider2D",Action::AddCollider},{"Add Rigidbody2D",Action::AddRigidbody}}},
        {"Services", {{"Services are not configured",Action::None}}},
        {"Tools", {{"Tilemap Editor",Action::ToggleTilemap},{"UI Editor",Action::ToggleUIEditor}}},
        {"Jobs", {{"No background jobs",Action::None}}},
        {"Window", {{"Hierarchy",Action::ToggleHierarchy},{"Inspector",Action::ToggleInspector},{"Animation",Action::ToggleAnimationWindow},{"Animator",Action::ToggleAnimatorWindow},{"Project Settings...",Action::ToggleProjectSettings},{"Render Settings...",Action::ToggleRenderSettings},{"Build Settings...",Action::BuildSettings},{"Asset Browser",Action::ToggleAssetBrowser},{"Debug Overlay",Action::ToggleDebug}}},
        {"Help", {{"About",Action::About}}}
    };
    float x = 5.0f;
    for (const auto& definition : definitions)
    {
        const float width = static_cast<float>(std::strlen(definition.first) * 8 + 18);
        menus.push_back({definition.first, x, width, definition.second});
        x += width;
    }
}

TopMenuBar::Action TopMenuBar::Update()
{
    consumedClick = false;
    InputManager& input = InputManager::GetInstance();
    const Vector2 mouse = input.GetMousePosition();
    if (!input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return Action::None;
    if (activeMenu >= 0) consumedClick = true;
    if (mouse.y <= 27.0f)
    {
        for (int i=0;i<static_cast<int>(menus.size());++i)
            if (mouse.x >= menus[i].x && mouse.x <= menus[i].x + menus[i].width) { activeMenu = activeMenu == i ? -1 : i; return Action::None; }
    }
    if (activeMenu >= 0)
    {
        const Menu& menu = menus[static_cast<std::size_t>(activeMenu)];
        const Rectangle dropdown{menu.x, 27.0f, 220.0f, static_cast<float>(menu.items.size()) * 26.0f};
        if (CheckCollisionPointRec(mouse, dropdown))
        {
            const int item = static_cast<int>((mouse.y - dropdown.y) / 26.0f);
            activeMenu = -1;
            if (item >= 0 && item < static_cast<int>(menu.items.size()))
            {
                const Action action = menu.items[static_cast<std::size_t>(item)].action;
                if (action == Action::Quit) wantsExit = true;
                consumedClick = true;
                return action;
            }
        }
        activeMenu = -1;
    }
    return Action::None;
}

void TopMenuBar::Draw() const
{
    DrawRectangle(0,0,GetScreenWidth(),27,Color{195,195,195,255});
    for (std::size_t i=0;i<menus.size();++i)
    {
        const Menu& menu=menus[i]; const Rectangle rect{menu.x,0,menu.width,27};
        if (activeMenu == static_cast<int>(i)) DrawRectangleRec(rect,Color{110,155,215,255});
        DrawText(menu.title,static_cast<int>(menu.x+9),6,15,BLACK);
    }
    if (activeMenu >= 0)
    {
        const Menu& menu=menus[static_cast<std::size_t>(activeMenu)];
        Rectangle box{menu.x,27,220,static_cast<float>(menu.items.size())*26};
        DrawRectangleRec(box,Color{235,235,235,255}); DrawRectangleLinesEx(box,1,DARKGRAY);
        for (std::size_t i=0;i<menu.items.size();++i)
        {
            Rectangle row{box.x,box.y+static_cast<float>(i)*26,box.width,26};
            if (CheckCollisionPointRec(GetMousePosition(),row)) DrawRectangleRec(row,Color{175,205,240,255});
            DrawText(menu.items[i].label,static_cast<int>(row.x+10),static_cast<int>(row.y+7),13,BLACK);
        }
    }
    if (!status.empty()) DrawText(status.c_str(),GetScreenWidth()-220,7,12,DARKGRAY);
}
bool TopMenuBar::BlocksMouse() const
{
    if (consumedClick) return true;
    const Vector2 mouse=InputManager::GetInstance().GetMousePosition();
    if (mouse.y <= 27.0f) return true;
    if (activeMenu < 0) return false;
    return true;
}
bool TopMenuBar::WantsExit() const { return wantsExit; }
