#include "UIEditor.h"
#include "../../Engine/UI/UIComponent.h"
#include "../../Engine/Core/InputManager.h"

GameObject* UIEditor::EnsureCanvas()
{
    for (auto& object : scene.GetGameObjects()) if (object && object->GetComponent<UICanvas>()) return object.get();
    return nullptr;
}
void UIEditor::Update()
{
    InputManager& input = InputManager::GetInstance();
    if (input.IsKeyPressed(KEY_F11)) visible = !visible;
    if (!visible || !input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    const Vector2 mouse = input.GetMousePosition();
    if (mouse.y >= 58.0f && mouse.y < 96.0f)
    {
        const int item = static_cast<int>((mouse.x - 82.0f) / 68.0f);
        if (item >= 0 && item <= 5) AddControl(item == 0 ? 4 : item == 1 ? 0 : item == 2 ? 1 : item == 3 ? 2 : item == 4 ? 3 : 5);
    }
}
void UIEditor::AddControl(int type)
{
    if (type == 4)
    {
        GameObject* canvas = scene.CreateGameObject("Canvas");
        if (canvas != nullptr) { canvas->AddComponent<UICanvas>(); canvas->AddComponent<RectTransformComponent>(); scene.SelectObject(canvas); status.clear(); }
        return;
    }
    GameObject* canvas = EnsureCanvas();
    if (canvas == nullptr) { status = "Create a Canvas before adding UI components."; return; }
    status.clear();
    GameObject* parent = scene.GetSelectedObject();
    GameObject* root = parent;
    while (root != nullptr && root->GetParent() != nullptr) root = root->GetParent();
    if (root != canvas) parent = canvas;
    if (type == 5) { if (parent->GetComponent<UILayoutGroup>() == nullptr) parent->AddComponent<UILayoutGroup>(); return; }
    GameObject* object = scene.CreateGameObject(type == 0 ? "UI Text" : type == 1 ? "UI Button" : type == 2 ? "UI Image" : type == 3 ? "UI Input" : "Canvas");
    if (object == nullptr) return;
    object->SetParent(parent);
    object->AddComponent<RectTransformComponent>();
    if (type == 0) object->AddComponent<UITextComponent>();
    else if (type == 1) object->AddComponent<UIButtonComponent>();
    else if (type == 2) object->AddComponent<UIImageComponent>();
    else if (type == 3) object->AddComponent<UIInputField>();
    scene.SelectObject(object);
    if (type == 1 || type == 3)
    {
        bool hasEventSystem = false;
        for (const auto& candidate : scene.GetGameObjects())
            if (candidate && candidate->GetComponent<UIEventSystem>()) { hasEventSystem = true; break; }
        if (!hasEventSystem)
        {
            GameObject* eventSystem = scene.CreateGameObject("EventSystem");
            if (eventSystem) eventSystem->AddComponent<UIEventSystem>();
        }
    }
}
void UIEditor::CreateElement(int type)
{
    if (type == 0) { AddControl(4); return; }
    if (type == 1)
    {
        GameObject* eventSystem = scene.CreateGameObject("EventSystem");
        if (eventSystem) { eventSystem->AddComponent<UIEventSystem>(); scene.SelectObject(eventSystem); }
        return;
    }
    if (type == 2) { AddControl(2); return; }
    if (type == 3) { AddControl(0); return; }
    if (type == 4) { AddControl(1); return; }
    if (type == 5) { AddControl(3); return; }
    if (type == 6)
    {
        GameObject* canvas = EnsureCanvas();
        if (!canvas) { status = "Create a Canvas before adding UI components."; return; }
        GameObject* panel = scene.CreateGameObject("UI Panel");
        if (!panel) return;
        panel->SetParent(canvas); panel->AddComponent<RectTransformComponent>(); panel->AddComponent<UIImageComponent>();
        scene.SelectObject(panel); return;
    }
    if (type == 7) AddControl(5);
}
void UIEditor::Draw() const
{
    if (!visible) return;
    DrawRectangle(0, 58, 500, 38, Color{20,24,32,245});
    DrawText("UI Editor", 8, 70, 14, SKYBLUE);
    const char* labels[] = {"Canvas", "Text", "Button", "Image", "Input", "Layout"};
    bool hasCanvas = false;
    for (const auto& object : scene.GetGameObjects()) if (object && object->GetComponent<UICanvas>()) { hasCanvas = true; break; }
    for (int i=0;i<6;++i) { Rectangle r{82.0f+i*68.0f, 62, 64, 30}; DrawRectangleRec(r, i == 0 || hasCanvas ? DARKGRAY : Color{55,55,55,255}); DrawText(labels[i], (int)r.x+5, 70, 11, i == 0 || hasCanvas ? WHITE : GRAY); }
    if (!status.empty()) DrawText(status.c_str(), 500, 71, 12, YELLOW);
}
bool UIEditor::IsMouseOver() const { return visible && CheckCollisionPointRec(InputManager::GetInstance().GetMousePosition(), {0,58,500,38}); }
