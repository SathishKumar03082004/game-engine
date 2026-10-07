#include "Scene.h"

#include <algorithm>

Scene::Scene()
{
    selectedObject = nullptr;

    gridSize = 64;
    gridExtent = 15000;
}

void Scene::Initialize()
{
    GameObject* player =
        CreateGameObject("Player");

    if (player != nullptr)
    {
        player->GetTransform().SetPosition(
            { 200.0f, 200.0f }
        );
    }
}

void Scene::Update()
{
    for (auto& object : gameObjects)
    {
        object->Update();
    }
}

void Scene::Draw()
{
    DrawGrid();

    for (auto& object : gameObjects)
    {
        if (object->GetParent() == nullptr)
        {
            object->Draw();
        }
    }
}

void Scene::DrawGrid()
{
    for (
        int x = -gridExtent;
        x <= gridExtent;
        x += gridSize
    )
    {
        DrawLine(
            x,
            -gridExtent,
            x,
            gridExtent,
            LIGHTGRAY
        );
    }

    for (
        int y = -gridExtent;
        y <= gridExtent;
        y += gridSize
    )
    {
        DrawLine(
            -gridExtent,
            y,
            gridExtent,
            y,
            LIGHTGRAY
        );
    }

    DrawLineEx(
        {
            static_cast<float>(-gridExtent),
            0.0f
        },
        {
            static_cast<float>(gridExtent),
            0.0f
        },
        3.0f,
        RED
    );

    DrawLineEx(
        {
            0.0f,
            static_cast<float>(-gridExtent)
        },
        {
            0.0f,
            static_cast<float>(gridExtent)
        },
        3.0f,
        BLUE
    );

    DrawCircle(
        0.0f,
        0.0f,
        6.0f,
        BLACK
    );

    DrawText(
        "0,0",
        10,
        10,
        18,
        BLACK
    );
}

void Scene::SelectObject(
    Vector2 worldPosition
)
{
    selectedObject = nullptr;

    for (auto it = gameObjects.rbegin();
         it != gameObjects.rend();
         ++it)
    {
        if ((*it)->ContainsPoint(worldPosition))
        {
            selectedObject = it->get();
            break;
        }
    }

    for (auto& object : gameObjects)
    {
        object->SetSelected(
            object.get() == selectedObject
        );
    }
}

void Scene::SelectObject(
    GameObject* object
)
{
    if (object == nullptr)
    {
        selectedObject = nullptr;

        for (auto& gameObject : gameObjects)
        {
            gameObject->SetSelected(false);
        }

        return;
    }

    selectedObject = object;

    for (auto& gameObject : gameObjects)
    {
        gameObject->SetSelected(
            gameObject.get() == selectedObject
        );
    }
}

GameObject* Scene::GetSelectedObject()
{
    return selectedObject;
}

std::vector<std::unique_ptr<GameObject>>&
Scene::GetGameObjects()
{
    return gameObjects;
}

GameObject* Scene::CreateGameObject(
    const std::string& name
)
{
    auto object =
        std::make_unique<GameObject>();

    object->SetName(name);

    GameObject* objectPointer =
        object.get();

    gameObjects.push_back(
        std::move(object)
    );

    return objectPointer;
}

std::string Scene::GenerateDuplicateName(
    const std::string& originalName
) const
{
    std::string baseName = originalName;

    int number = 1;

    while (true)
    {
        std::string candidate =
            baseName +
            " (" +
            std::to_string(number) +
            ")";

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

        number++;
    }
}

GameObject* Scene::DuplicateGameObject(
    GameObject* object
)
{
    if (object == nullptr)
    {
        return nullptr;
    }

    GameObject* parent =
        object->GetParent();

    return DuplicateRecursive(
        object,
        parent
    );
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

    std::string newName =
        GenerateDuplicateName(
            source->GetName()
        );

    GameObject* duplicate =
        CreateGameObject(newName);

    if (duplicate == nullptr)
    {
        return nullptr;
    }

    duplicate->GetTransform() =
        source->GetTransform();

    if (parent != nullptr)
    {
        SetParent(
            duplicate,
            parent
        );
    }

    for (GameObject* child :
         source->GetChildren())
    {
        DuplicateRecursive(
            child,
            duplicate
        );
    }

    return duplicate;
}

void Scene::DestroyGameObject(
    GameObject* object
)
{
    if (object == nullptr)
    {
        return;
    }

    if (selectedObject == object)
    {
        selectedObject = nullptr;
    }

    if (object->GetParent() != nullptr)
    {
        object->GetParent()->RemoveChild(
            object
        );
    }

    for (GameObject* child :
         object->GetChildren())
    {
        child->SetParent(nullptr);
    }

    auto it =
        std::find_if(
            gameObjects.begin(),
            gameObjects.end(),
            [object](
                const std::unique_ptr<GameObject>& item
            )
            {
                return item.get() == object;
            }
        );

    if (it != gameObjects.end())
    {
        gameObjects.erase(it);
    }
}

bool Scene::WouldCreateCycle(
    GameObject* child,
    GameObject* potentialParent
) const
{
    if (
        child == nullptr ||
        potentialParent == nullptr
    )
    {
        return false;
    }

    if (child == potentialParent)
    {
        return true;
    }

    GameObject* current =
        potentialParent->GetParent();

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

void Scene::SetParent(
    GameObject* child,
    GameObject* parent
)
{
    if (child == nullptr)
    {
        return;
    }

    if (parent == nullptr)
    {
        ClearParent(child);
        return;
    }

    if (WouldCreateCycle(
        child,
        parent
    ))
    {
        return;
    }

    child->SetParent(parent);
}

void Scene::ClearParent(
    GameObject* child
)
{
    if (child == nullptr)
    {
        return;
    }

    child->SetParent(nullptr);
}

void Scene::DragSelectedObject(
    Vector2 worldPosition
)
{
    if (selectedObject == nullptr)
    {
        return;
    }

    GameObject* parent =
        selectedObject->GetParent();

    if (parent == nullptr)
    {
        selectedObject
            ->GetTransform()
            .SetPosition(worldPosition);

        return;
    }

    Vector2 parentPosition =
        parent->GetWorldPosition();

    Vector2 localPosition =
    {
        worldPosition.x - parentPosition.x,
        worldPosition.y - parentPosition.y
    };

    selectedObject
        ->GetTransform()
        .SetPosition(localPosition);
}