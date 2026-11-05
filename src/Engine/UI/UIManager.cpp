#include "UIManager.h"
#include "UIComponent.h"
#include "../Core/InputManager.h"
#include <algorithm>
#include <vector>

namespace
{
    bool IsWithinCanvas(const GameObject* object)
    {
        if (object == nullptr) return false;
        const GameObject* root = object;
        while (root->GetParent() != nullptr) root = root->GetParent();
        const UICanvas* canvas = root->GetComponent<UICanvas>();
        return canvas != nullptr && canvas->IsEnabled();
    }

    void ApplyLayoutGroups(Scene& scene)
    {
        for (const auto& parent : scene.GetGameObjects())
        {
            if (!parent || !IsWithinCanvas(parent.get())) continue;
            const UILayoutGroup* group = parent->GetComponent<UILayoutGroup>();
            if (!group) continue;
            Rectangle parentRect{0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
            if (!parent->GetComponent<UICanvas>())
                if (const UIComponent* parentUI = parent->GetComponent<UIComponent>()) parentRect = parentUI->GetScreenRect();
            Vector2 cursor{parentRect.x + group->padding, parentRect.y + group->padding};
            for (GameObject* child : parent->GetChildren())
            {
                if (!child) continue;
                UIComponent* ui = child->GetComponent<UIComponent>();
                if (!ui) continue;
                ui->SetLayoutRect({cursor.x, cursor.y, group->itemSize.x, group->itemSize.y});
                if (group->direction == UILayoutGroup::Direction::Vertical) cursor.y += group->itemSize.y + group->spacing;
                else cursor.x += group->itemSize.x + group->spacing;
            }
        }
    }
}

void UIManager::Update(Scene& scene)
{
    bool hasEventSystem = false;
    for (const auto& object : scene.GetGameObjects())
        if (object && object->GetComponent<UIEventSystem>() && object->GetComponent<UIEventSystem>()->IsEnabled()) { hasEventSystem = true; break; }
    if (!hasEventSystem) { keyboardFocus = nullptr; return; }
    bool hasCanvas = false;
    for (auto& object : scene.GetGameObjects())
        if (object && object->GetComponent<UICanvas>() && object->GetComponent<UICanvas>()->IsEnabled()) hasCanvas = true;
    if (!hasCanvas) return;
    ApplyLayoutGroups(scene);
    if (InputManager::GetInstance().IsKeyPressed(KEY_TAB))
    {
        std::vector<GameObject*> focusables;
        for (const auto& object : scene.GetGameObjects())
        {
            if (!object || !IsWithinCanvas(object.get())) continue;
            if (object->GetComponent<UIButtonComponent>() || object->GetComponent<UIInputField>()) focusables.push_back(object.get());
        }
        if (!focusables.empty())
        {
            auto it = std::find(focusables.begin(), focusables.end(), keyboardFocus);
            const std::size_t next = it == focusables.end() ? 0 : (static_cast<std::size_t>(it - focusables.begin()) + 1) % focusables.size();
            keyboardFocus = focusables[next];
            for (GameObject* object : focusables)
            {
                if (auto* button = object->GetComponent<UIButtonComponent>()) button->focused = object == keyboardFocus;
                if (auto* input = object->GetComponent<UIInputField>()) input->focused = object == keyboardFocus;
            }
        }
    }
    for (auto& object : scene.GetGameObjects())
    {
        if (!object || !IsWithinCanvas(object.get())) continue;
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
    ApplyLayoutGroups(scene);
    std::vector<GameObject*> orderedObjects;
    for (const auto& object : scene.GetGameObjects()) if (object && IsWithinCanvas(object.get())) orderedObjects.push_back(object.get());
    std::stable_sort(orderedObjects.begin(), orderedObjects.end(), [](const GameObject* a, const GameObject* b)
    {
        const GameObject* rootA = a; while (rootA->GetParent()) rootA = rootA->GetParent();
        const GameObject* rootB = b; while (rootB->GetParent()) rootB = rootB->GetParent();
        return rootA->GetComponent<UICanvas>()->sortingOrder < rootB->GetComponent<UICanvas>()->sortingOrder;
    });
    for (GameObject* object : orderedObjects)
    {
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

GameObject* UIManager::GetObjectAtPoint(Scene& scene, Vector2 screenPoint) const
{
    const auto& objects = scene.GetGameObjects();
    for (auto object = objects.rbegin(); object != objects.rend(); ++object)
    {
        if (!*object || !IsWithinCanvas(object->get())) continue;
        UIComponent* ui = (*object)->GetComponent<UIComponent>();
        if (ui != nullptr && CheckCollisionPointRec(screenPoint, ui->GetScreenRect())) return object->get();
    }
    return nullptr;
}
