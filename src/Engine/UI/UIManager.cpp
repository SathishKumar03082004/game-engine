#include "UIManager.h"
#include "UIComponent.h"

void UIManager::Update(Scene& scene)
{
    bool hasCanvas = false;
    for (auto& object : scene.GetGameObjects())
        if (object && object->GetComponent<UICanvas>() && object->GetComponent<UICanvas>()->IsEnabled()) hasCanvas = true;
    if (!hasCanvas) return;
    for (auto& object : scene.GetGameObjects())
    {
        if (!object) continue;
        for (const auto& component : object->GetComponents())
        {
            if (!component || !component->IsEnabled()) continue;
            if (auto* button = dynamic_cast<UIButtonComponent*>(component.get())) button->Update(GetFrameTime());
            else if (auto* input = dynamic_cast<UIInputField*>(component.get())) input->Update(GetFrameTime());
        }
    }
}
void UIManager::Draw(Scene& scene)
{
    bool hasCanvas = false;
    for (const auto& object : scene.GetGameObjects())
        if (object && object->GetComponent<UICanvas>() && object->GetComponent<UICanvas>()->IsEnabled()) hasCanvas = true;
    if (!hasCanvas) return;
    for (auto& parent : scene.GetGameObjects())
    {
        if (!parent) continue;
        const UILayoutGroup* group = parent->GetComponent<UILayoutGroup>();
        if (!group) continue;
        Vector2 start = {group->padding, group->padding};
        for (GameObject* child : parent->GetChildren())
        {
            if (!child) continue;
            UIComponent* ui = nullptr;
            for (const auto& component : child->GetComponents()) if ((ui = dynamic_cast<UIComponent*>(component.get())) != nullptr) break;
            if (!ui) continue;
            Rectangle rect = ui->GetScreenRect();
            rect.x = start.x; rect.y = start.y; rect.width = group->itemSize.x; rect.height = group->itemSize.y;
            ui->SetLayoutRect(rect);
            if (group->direction == UILayoutGroup::Direction::Vertical) start.y += group->itemSize.y + group->spacing;
            else start.x += group->itemSize.x + group->spacing;
        }
    }
    for (const auto& object : scene.GetGameObjects())
    {
        if (!object) continue;
        for (const auto& component : object->GetComponents())
        {
            if (!component || !component->IsEnabled()) continue;
            if (auto* image = dynamic_cast<UIImageComponent*>(component.get())) image->Draw();
            else if (auto* button = dynamic_cast<UIButtonComponent*>(component.get())) button->Draw();
            else if (auto* text = dynamic_cast<UITextComponent*>(component.get())) text->Draw();
            else if (auto* bar = dynamic_cast<UIProgressBar*>(component.get())) bar->Draw();
            else if (auto* input = dynamic_cast<UIInputField*>(component.get())) input->Draw();
        }
    }
}
void UIManager::ReleaseResources(Scene& scene)
{
    for (const auto& object : scene.GetGameObjects())
    {
        if (!object) continue;
        if (auto* image = object->GetComponent<UIImageComponent>()) image->Unload();
    }
}
