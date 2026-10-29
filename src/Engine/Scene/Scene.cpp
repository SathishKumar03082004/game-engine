#include "Scene.h"
#include <algorithm>

Scene::Scene()
    : selectedObject(nullptr),
      gridSize(64),
      gridExtent(15000)
{
}

void Scene::Initialize()
{
    GameObject* player = CreateGameObject("Player");

    if (player != nullptr)
    {
        player->GetTransform().SetPosition({200.0f, 200.0f});
    }

    GameObject* enemy = CreateGameObject("Enemy");

    if (enemy != nullptr)
    {
        enemy->GetTransform().SetPosition({400.0f, 200.0f});
    }

    GameObject* platform = CreateGameObject("Platform");

    if (platform != nullptr)
    {
        platform->GetTransform().SetPosition({600.0f, 200.0f});
    }
}

void Scene::Update()
{
    for (auto& object : gameObjects)
    {
        if (object != nullptr && object->GetParent() == nullptr)
        {
            object->Update();
        }
    }
}

void Scene::Draw()
{
    DrawGrid();

    for (auto& object : gameObjects)
    {
        if (object != nullptr && object->GetParent() == nullptr)
        {
            object->Draw();
        }
    }
}

void Scene::DrawGrid()
{
    for (int x = -gridExtent; x <= gridExtent; x += gridSize)
    {
        DrawLine(x, -gridExtent, x, gridExtent, LIGHTGRAY);
    }

    for (int y = -gridExtent; y <= gridExtent; y += gridSize)
    {
        DrawLine(-gridExtent, y, gridExtent, y, LIGHTGRAY);
    }

    DrawLineEx(
        {-static_cast<float>(gridExtent), 0.0f},
        {static_cast<float>(gridExtent), 0.0f},
        3.0f,
        RED
    );

    DrawLineEx(
        {0.0f, -static_cast<float>(gridExtent)},
        {0.0f, static_cast<float>(gridExtent)},
        3.0f,
        BLUE
    );

    DrawCircle(0.0f, 0.0f, 6.0f, BLACK);
    DrawText("0,0", 10, 10, 18, BLACK);
}

bool Scene::ContainsGameObject(GameObject* object) const
{
    if (object == nullptr)
    {
        return false;
    }

    return std::any_of(
        gameObjects.begin(),
        gameObjects.end(),
        [object](const std::unique_ptr<GameObject>& item)
        {
            return item.get() == object;
        }
    );
}

void Scene::SelectObject(Vector2 worldPosition, bool ctrlPressed)
{
    GameObject* clickedObject = nullptr;

    for (auto it = gameObjects.rbegin(); it != gameObjects.rend(); ++it)
    {
        if ((*it)->ContainsPoint(worldPosition))
        {
            clickedObject = it->get();
            break;
        }
    }

    if (clickedObject == nullptr)
    {
        ClearSelection();
        return;
    }

    SelectObject(clickedObject, ctrlPressed);
}

void Scene::SelectObject(GameObject* object, bool ctrlPressed)
{
    if (object == nullptr)
    {
        ClearSelection();
        return;
    }

    if (!ContainsGameObject(object))
    {
        return;
    }

    if (ctrlPressed)
    {
        ToggleSelection(object);
        return;
    }

    ClearSelection();

    selectedObjects.push_back(object);
    selectedObject = object;

    object->SetSelected(true);
}

void Scene::ToggleSelection(GameObject* object)
{
    if (!ContainsGameObject(object))
    {
        return;
    }

    auto it = std::find(
        selectedObjects.begin(),
        selectedObjects.end(),
        object
    );

    if (it != selectedObjects.end())
    {
        selectedObjects.erase(it);
        object->SetSelected(false);

        if (selectedObject == object)
        {
            selectedObject = selectedObjects.empty()
                ? nullptr
                : selectedObjects.back();
        }

        return;
    }

    selectedObjects.push_back(object);
    selectedObject = object;
    object->SetSelected(true);
}

void Scene::ClearSelection()
{
    for (GameObject* object : selectedObjects)
    {
        if (object != nullptr)
        {
            object->SetSelected(false);
        }
    }

    selectedObjects.clear();
    selectedObject = nullptr;
}

GameObject* Scene::GetSelectedObject()
{
    return selectedObject;
}

const std::vector<GameObject*>& Scene::GetSelectedObjects() const
{
    return selectedObjects;
}

bool Scene::IsObjectSelected(GameObject* object) const
{
    return std::find(
        selectedObjects.begin(),
        selectedObjects.end(),
        object
    ) != selectedObjects.end();
}

std::vector<std::unique_ptr<GameObject>>& Scene::GetGameObjects()
{
    return gameObjects;
}

GameObject* Scene::CreateGameObject(const std::string& name)
{
    auto object = std::make_unique<GameObject>();
    object->SetName(name);

    GameObject* objectPointer = object.get();

    gameObjects.push_back(std::move(object));

    return objectPointer;
}

std::string Scene::GenerateDuplicateName(
    const std::string& originalName
) const
{
    int number = 1;

    while (true)
    {
        std::string candidate =
            originalName + " (" + std::to_string(number) + ")";

        bool exists = false;

        for (const auto& object : gameObjects)
        {
            if (object->GetName() == candidate)
            {
                exists = true;
                break;
            }
        }

        if (!exists)
        {
            return candidate;
        }

        ++number;
    }
}

GameObject* Scene::DuplicateGameObject(GameObject* object)
{
    if (!ContainsGameObject(object))
    {
        return nullptr;
    }

    return DuplicateRecursive(object, object->GetParent());
}

GameObject* Scene::DuplicateRecursive(
    GameObject* source,
    GameObject* parent
)
{
    if (source == nullptr)
    {
        return nullptr;
    }

    GameObject* duplicate =
        CreateGameObject(GenerateDuplicateName(source->GetName()));

    if (duplicate == nullptr)
    {
        return nullptr;
    }

    duplicate->GetTransform() = source->GetTransform();

    if (parent != nullptr)
    {
        SetParent(duplicate, parent);
    }

    std::vector<GameObject*> children = source->GetChildren();

    for (GameObject* child : children)
    {
        DuplicateRecursive(child, duplicate);
    }

    return duplicate;
}

void Scene::DestroyGameObject(GameObject* object)
{
    if (!ContainsGameObject(object))
    {
        return;
    }

    Vector2 worldPosition = object->GetWorldPosition();
    float worldRotation = object->GetWorldRotation();
    Vector2 worldScale = object->GetWorldScale();

    auto children = object->GetChildren();

    for (GameObject* child : children)
    {
        if (child == nullptr)
        {
            continue;
        }

        Vector2 childWorldPosition = child->GetWorldPosition();
        float childWorldRotation = child->GetWorldRotation();
        Vector2 childWorldScale = child->GetWorldScale();

        child->SetParent(nullptr);

        child->GetTransform().SetPosition(childWorldPosition);
        child->GetTransform().SetRotation(childWorldRotation);
        child->GetTransform().SetScale(childWorldScale);
    }

    if (object->GetParent() != nullptr)
    {
        object->GetParent()->RemoveChild(object);
    }

    selectedObjects.erase(
        std::remove(
            selectedObjects.begin(),
            selectedObjects.end(),
            object
        ),
        selectedObjects.end()
    );

    if (selectedObject == object)
    {
        selectedObject = selectedObjects.empty()
            ? nullptr
            : selectedObjects.back();
    }

    object->SetSelected(false);

    auto it = std::find_if(
        gameObjects.begin(),
        gameObjects.end(),
        [object](const std::unique_ptr<GameObject>& item)
        {
            return item.get() == object;
        }
    );

    if (it != gameObjects.end())
    {
        gameObjects.erase(it);
    }

    if (selectedObject != nullptr)
    {
        selectedObject->SetSelected(true);
    }

    static_cast<void>(worldPosition);
    static_cast<void>(worldRotation);
    static_cast<void>(worldScale);
}

bool Scene::WouldCreateCycle(
    GameObject* child,
    GameObject* potentialParent
) const
{
    if (child == nullptr || potentialParent == nullptr)
    {
        return false;
    }

    if (child == potentialParent)
    {
        return true;
    }

    GameObject* current = potentialParent;

    while (current != nullptr)
    {
        if (current == child)
        {
            return true;
        }

        current = current->GetParent();
    }

    return false;
}

void Scene::SetParent(GameObject* child, GameObject* parent)
{
    if (child == nullptr || !ContainsGameObject(child))
    {
        return;
    }

    if (parent == nullptr)
    {
        ClearParent(child);
        return;
    }

    if (!ContainsGameObject(parent) || WouldCreateCycle(child, parent))
    {
        return;
    }

    child->SetParent(parent);
}

void Scene::ClearParent(GameObject* child)
{
    if (child == nullptr || child->GetParent() == nullptr)
    {
        return;
    }

    Vector2 worldPosition = child->GetWorldPosition();
    float worldRotation = child->GetWorldRotation();
    Vector2 worldScale = child->GetWorldScale();

    child->SetParent(nullptr);

    child->GetTransform().SetPosition(worldPosition);
    child->GetTransform().SetRotation(worldRotation);
    child->GetTransform().SetScale(worldScale);
}

void Scene::DragSelectedObject(Vector2 worldPosition)
{
    if (selectedObject == nullptr)
    {
        return;
    }

    GameObject* parent = selectedObject->GetParent();

    Vector2 localPosition = worldPosition;

    if (parent != nullptr)
    {
        localPosition = parent->InverseTransformPoint(worldPosition);
    }

    selectedObject->GetTransform().SetPosition(localPosition);
}