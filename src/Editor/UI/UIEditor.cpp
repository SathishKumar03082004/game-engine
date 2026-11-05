#include "UIEditor.h"
#include "../../Engine/UI/UIComponent.h"
#include "../../Engine/Core/InputManager.h"

GameObject* UIEditor::EnsureCanvas()
{
    for (auto& object : scene.GetGameObjects()) if (object && object->GetComponent<UICanvas>()) return object.get();
    GameObject* canvas = scene.CreateGameObject("Canvas");
    if (canvas) canvas->AddComponent<UICanvas>();
    return canvas;
}
void UIEditor::Update()
{
    InputManager& input = InputManager::GetInstance();
    if (input.IsKeyPressed(KEY_F11)) visible = !visible;
    if (!visible || !input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    const Vector2 mouse = input.GetMousePosition();
    if (mouse.y < 38.0f)
    {
        const int item = static_cast<int>((mouse.x - 82.0f) / 68.0f);
        if (item >= 0 && item <= 5) AddControl(item == 0 ? 4 : item == 1 ? 0 : item == 2 ? 1 : item == 3 ? 2 : item == 4 ? 3 : 5);
    }
}
void UIEditor::AddControl(int type)
{
    GameObject* parent = scene.GetSelectedObject();
    if (parent == nullptr || parent->GetComponent<UICanvas>() == nullptr)
    {
        for (auto& object : scene.GetGameObjects()) if (object && object->GetComponent<UICanvas>()) { parent = object.get(); break; }
    }
    if (parent == nullptr) parent = EnsureCanvas();
    if (parent == nullptr) return;
    if (type == 4) { EnsureCanvas(); return; }
    if (type == 5) { if (parent->GetComponent<UILayoutGroup>() == nullptr) parent->AddComponent<UILayoutGroup>(); return; }
    GameObject* object = scene.CreateGameObject(type == 0 ? "UI Text" : type == 1 ? "UI Button" : type == 2 ? "UI Image" : type == 3 ? "UI Input" : "Canvas");
    if (object == nullptr) return;
    object->SetParent(parent);
    if (type == 0) object->AddComponent<UITextComponent>();
    else if (type == 1) object->AddComponent<UIButtonComponent>();
    else if (type == 2) object->AddComponent<UIImageComponent>();
    else if (type == 3) object->AddComponent<UIInputField>();
    scene.SelectObject(object);
}
void UIEditor::Draw() const
{
    if (!visible) return;
    DrawRectangle(0, 0, 500, 38, Color{20,24,32,245});
    DrawText("UI Editor", 8, 12, 14, SKYBLUE);
    const char* labels[] = {"Canvas", "Text", "Button", "Image", "Input", "Layout"};
    for (int i=0;i<6;++i) { Rectangle r{82.0f+i*68.0f, 4, 64, 30}; DrawRectangleRec(r, DARKGRAY); DrawText(labels[i], (int)r.x+5, 12, 11, WHITE); }
}
bool UIEditor::IsMouseOver() const { return visible && CheckCollisionPointRec(InputManager::GetInstance().GetMousePosition(), {0,0,500,38}); }
