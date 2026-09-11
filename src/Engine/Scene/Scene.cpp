#include "Scene.h"

#include <algorithm>
#include <string>

// =========================================================
// CONSTRUCTOR
// =========================================================

Scene::Scene()
{
    selectedObject = nullptr;

    gridSize = 64;

    gridExtent = 15000;
}

// =========================================================
// INITIALIZE
// =========================================================

void Scene::Initialize()
{
    // =========================================================
    // PLAYER
    // =========================================================

    GameObject* player = CreateGameObject("Player");

    player->GetTransform().SetPosition(
        { 200.0f, 200.0f }
    );

    player->GetTransform().SetRotation(
        45.0f
    );

    player->GetTransform().SetScale(
        { 1.5f, 1.5f }
    );

    // =========================================================
    // ENEMY
    // =========================================================

    GameObject* enemy = CreateGameObject("Enemy");

    enemy->GetTransform().SetPosition(
        { 500.0f, 300.0f }
    );

    // =========================================================
    // CAMERA
    // =========================================================

    GameObject* camera = CreateGameObject("Camera");

    camera->GetTransform().SetPosition(
        { 700.0f, 500.0f }
    );

    // =========================================================
    // EXAMPLE CHILD
    // =========================================================

    GameObject* weapon = CreateGameObject("Weapon");

    weapon->GetTransform().SetPosition(
        { 120.0f, 0.0f }
    );

    SetParent(
        weapon,
        player
    );

    // =========================================================
    // SELECT PLAYER
    // =========================================================

    SelectObject(player);
}

// =========================================================
// UPDATE
// =========================================================

void Scene::Update()
{
    for (auto& object : gameObjects)
    {
        object->Update();
    }
}

// =========================================================
// DRAW
// =========================================================

void Scene::Draw()
{
    DrawGrid();

    for (auto& object : gameObjects)
    {
        object->Draw();
    }
}

// =========================================================
// GRID
// =========================================================

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
}

// =========================================================
// SELECT BY WORLD POSITION
// =========================================================

void Scene::SelectObject(Vector2 worldPosition)
{
    GameObject* clickedObject = nullptr;

    // Search from back to front
    for (
        int i = static_cast<int>(gameObjects.size()) - 1;
        i >= 0;
        --i
    )
    {
        if (gameObjects[i]->ContainsPoint(worldPosition))
        {
            clickedObject = gameObjects[i].get();

            break;
        }
    }

    SelectObject(clickedObject);
}

// =========================================================
// SELECT POINTER
// =========================================================

void Scene::SelectObject(GameObject* object)
{
    if (selectedObject != nullptr)
    {
        selectedObject->SetSelected(false);
    }

    selectedObject = object;

    if (selectedObject != nullptr)
    {
        selectedObject->SetSelected(true);
    }
}

// =========================================================
// GET SELECTED OBJECT
// =========================================================

GameObject* Scene::GetSelectedObject()
{
    return selectedObject;
}

// =========================================================
// GET GAME OBJECTS
// =========================================================

std::vector<std::unique_ptr<GameObject>>&
Scene::GetGameObjects()
{
    return gameObjects;
}

// =========================================================
// CREATE GAME OBJECT
// =========================================================

GameObject* Scene::CreateGameObject(
    const std::string& requestedName
)
{
    std::string finalName = requestedName;

    int counter = 1;

    bool nameExists = true;

    while (nameExists)
    {
        nameExists = false;

        for (auto& object : gameObjects)
        {
            if (object->GetName() == finalName)
            {
                nameExists = true;

                finalName =
                    requestedName +
                    " " +
                    std::to_string(counter);

                counter++;

                break;
            }
        }
    }

    auto newObject =
        std::make_unique<GameObject>();

    newObject->SetName(finalName);

    float positionX =
        400.0f +
        static_cast<float>(gameObjects.size() * 50);

    float positionY =
        400.0f +
        static_cast<float>(gameObjects.size() * 30);

    newObject->GetTransform().SetPosition(
        { positionX, positionY }
    );

    gameObjects.push_back(
        std::move(newObject)
    );

    GameObject* createdObject =
        gameObjects.back().get();

    SelectObject(createdObject);

    return createdObject;
}

// =========================================================
// DESTROY GAME OBJECT
// =========================================================

void Scene::DestroyGameObject(
    GameObject* object
)
{
    if (object == nullptr)
    {
        return;
    }

    // ---------------------------------------------------------
    // Remove children from this parent
    // ---------------------------------------------------------

    const auto children =
        object->GetChildren();

    for (GameObject* child : children)
    {
        child->SetParent(nullptr);
    }

    // ---------------------------------------------------------
    // Remove from parent
    // ---------------------------------------------------------

    if (object->GetParent() != nullptr)
    {
        object->SetParent(nullptr);
    }

    // ---------------------------------------------------------
    // Clear selection
    // ---------------------------------------------------------

    if (selectedObject == object)
    {
        selectedObject = nullptr;
    }

    // ---------------------------------------------------------
    // Find and erase
    // ---------------------------------------------------------

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
}

// =========================================================
// SET PARENT
// =========================================================

void Scene::SetParent(
    GameObject* child,
    GameObject* parent
)
{
    if (child == nullptr)
    {
        return;
    }

    if (child == parent)
    {
        return;
    }

    if (WouldCreateCycle(child, parent))
    {
        return;
    }

    child->SetParent(parent);
}

// =========================================================
// CLEAR PARENT
// =========================================================

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

// =========================================================
// CHECK PARENT CYCLE
// =========================================================

bool Scene::WouldCreateCycle(
    GameObject* child,
    GameObject* potentialParent
) const
{
    if (potentialParent == nullptr)
    {
        return false;
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

// =========================================================
// DRAG SELECTED OBJECT
// =========================================================

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
        selectedObject->GetTransform().SetPosition(
            worldPosition
        );

        return;
    }

    // Convert world position into
    // simple local position.

    Vector2 parentWorld =
        parent->GetWorldPosition();

    Vector2 localPosition =
    {
        worldPosition.x - parentWorld.x,
        worldPosition.y - parentWorld.y
    };

    selectedObject->GetTransform().SetPosition(
        localPosition
    );
}