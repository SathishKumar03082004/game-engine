#include "Scene.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <unordered_map>
#include <utility>

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

    std::vector<GameObject*> sortedObjects;
    sortedObjects.reserve(gameObjects.size());

    for (const auto& object : gameObjects)
    {
        if (object != nullptr)
        {
            sortedObjects.push_back(object.get());
        }
    }

    std::stable_sort(
        sortedObjects.begin(),
        sortedObjects.end(),
        [](const GameObject* a, const GameObject* b)
        {
            if (a->GetRenderLayer() != b->GetRenderLayer())
            {
                return static_cast<int>(a->GetRenderLayer()) <
                       static_cast<int>(b->GetRenderLayer());
            }

            return a->GetSortingOrder() < b->GetSortingOrder();
        }
    );

    for (GameObject* object : sortedObjects)
    {
        object->Draw();
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
    std::vector<GameObject*> sortedObjects;
    sortedObjects.reserve(gameObjects.size());

    for (const auto& object : gameObjects)
    {
        if (object != nullptr)
        {
            sortedObjects.push_back(object.get());
        }
    }

    std::stable_sort(
        sortedObjects.begin(),
        sortedObjects.end(),
        [](const GameObject* a, const GameObject* b)
        {
            if (a->GetRenderLayer() != b->GetRenderLayer())
            {
                return static_cast<int>(a->GetRenderLayer()) <
                       static_cast<int>(b->GetRenderLayer());
            }

            return a->GetSortingOrder() < b->GetSortingOrder();
        }
    );

    GameObject* clickedObject = nullptr;

    for (auto it = sortedObjects.rbegin();
         it != sortedObjects.rend();
         ++it)
    {
        if ((*it)->ContainsPoint(worldPosition))
        {
            clickedObject = *it;
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
    duplicate->SetRenderLayer(source->GetRenderLayer());
    duplicate->SetSortingOrder(source->GetSortingOrder());

    if (source->GetSpriteRenderer().IsLoaded())
    {
        duplicate->SetSprite(source->GetSpriteRenderer().GetFilePath());

        duplicate->GetSpriteRenderer().SetTint(source->GetSpriteRenderer().GetTint());
    }

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

    if (child->GetParent() == parent)
    {
        return;
    }

    // Reparenting in the editor should change hierarchy without moving the
    // object in world space.
    const Vector2 worldPosition = child->GetWorldPosition();
    const float worldRotation = child->GetWorldRotation();
    const Vector2 worldScale = child->GetWorldScale();

    child->SetParent(parent);

    child->GetTransform().SetPosition(parent->InverseTransformPoint(worldPosition));
    child->GetTransform().SetRotation(worldRotation - parent->GetWorldRotation());

    const Vector2 parentScale = parent->GetWorldScale();
    child->GetTransform().SetScale({
        fabsf(parentScale.x) > 0.0001f ? worldScale.x / parentScale.x : worldScale.x,
        fabsf(parentScale.y) > 0.0001f ? worldScale.y / parentScale.y : worldScale.y
    });
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

bool Scene::SaveToFile(const std::string& path) const
{
    if (path.empty())
    {
        return false;
    }

    const std::filesystem::path filePath(path);
    std::error_code error;
    if (!filePath.parent_path().empty())
    {
        std::filesystem::create_directories(filePath.parent_path(), error);
        if (error)
        {
            return false;
        }
    }

    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file)
    {
        return false;
    }

    std::unordered_map<const GameObject*, std::size_t> objectIndices;
    objectIndices.reserve(gameObjects.size());
    for (std::size_t index = 0; index < gameObjects.size(); ++index)
    {
        if (gameObjects[index] != nullptr)
        {
            objectIndices.emplace(gameObjects[index].get(), index);
        }
    }

    file << "RAYLIB_SCENE 1\n";
    file << gameObjects.size() << '\n';
    file << std::setprecision(std::numeric_limits<float>::max_digits10);

    for (std::size_t index = 0; index < gameObjects.size(); ++index)
    {
        GameObject* object = gameObjects[index].get();
        if (object == nullptr)
        {
            file << "NULL\n";
            continue;
        }

        std::ptrdiff_t parentIndex = -1;
        if (object->GetParent() != nullptr)
        {
            const auto parent = objectIndices.find(object->GetParent());
            if (parent == objectIndices.end())
            {
                return false;
            }
            parentIndex = static_cast<std::ptrdiff_t>(parent->second);
        }

        const TransformComponent& transform = object->GetTransform();
        const Vector2 position = transform.GetPosition();
        const Vector2 scale = transform.GetScale();
        const AnimationComponent& animation = object->GetAnimation();
        const SpriteRenderer& sprite = object->GetSpriteRenderer();

        file << "OBJECT " << parentIndex << ' '
             << std::quoted(object->GetName()) << ' '
             << position.x << ' ' << position.y << ' '
             << transform.GetRotation() << ' '
             << scale.x << ' ' << scale.y << ' '
             << static_cast<int>(object->GetRenderLayer()) << ' '
             << object->GetSortingOrder() << ' '
             << std::quoted(sprite.GetFilePath()) << ' '
             << (animation.IsLoaded() ? 1 : 0) << ' '
             << std::quoted(animation.GetFilePath()) << ' '
             << animation.GetFrameWidth() << ' '
             << animation.GetFrameHeight() << ' '
             << animation.GetFrameCount() << ' '
             << animation.GetCurrentFrame() << ' '
             << animation.GetFPS() << ' '
             << (animation.IsLooping() ? 1 : 0) << ' '
             << (animation.IsPlaying() ? 1 : 0) << '\n';

        if (!file)
        {
            return false;
        }
    }

    file.flush();
    return file.good();
}

bool Scene::LoadFromFile(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }

    struct SavedObject
    {
        bool isNull = false;
        long long parentIndex = -1;
        std::string name;
        Vector2 position{};
        float rotation = 0.0f;
        Vector2 scale{1.0f, 1.0f};
        int renderLayer = static_cast<int>(RenderLayer::Default);
        int sortingOrder = 0;
        std::string spritePath;
        bool animationLoaded = false;
        std::string animationPath;
        int frameWidth = 0;
        int frameHeight = 0;
        int frameCount = 0;
        int currentFrame = 0;
        float animationFPS = 8.0f;
        bool looping = true;
        bool playing = false;
    };

    std::ifstream file(path);
    std::string magic;
    int version = 0;
    std::size_t objectCount = 0;
    if (!(file >> magic >> version >> objectCount) ||
        magic != "RAYLIB_SCENE" || version != 1 || objectCount > 100000)
    {
        return false;
    }

    std::vector<SavedObject> savedObjects(objectCount);
    for (std::size_t index = 0; index < objectCount; ++index)
    {
        std::string recordType;
        if (!(file >> recordType))
        {
            return false;
        }

        SavedObject& object = savedObjects[index];
        if (recordType == "NULL")
        {
            object.isNull = true;
            continue;
        }
        if (recordType != "OBJECT")
        {
            return false;
        }

        int animationLoaded = 0;
        int looping = 0;
        int playing = 0;
        if (!(file >> object.parentIndex
                  >> std::quoted(object.name)
                  >> object.position.x >> object.position.y
                  >> object.rotation
                  >> object.scale.x >> object.scale.y
                  >> object.renderLayer
                  >> object.sortingOrder
                  >> std::quoted(object.spritePath)
                  >> animationLoaded
                  >> std::quoted(object.animationPath)
                  >> object.frameWidth
                  >> object.frameHeight
                  >> object.frameCount
                  >> object.currentFrame
                  >> object.animationFPS
                  >> looping
                  >> playing))
        {
            return false;
        }

        object.animationLoaded = animationLoaded != 0;
        object.looping = looping != 0;
        object.playing = playing != 0;

        if (object.parentIndex < -1 ||
            object.parentIndex >= static_cast<long long>(objectCount) ||
            object.parentIndex == static_cast<long long>(index) ||
            object.renderLayer < static_cast<int>(RenderLayer::Background) ||
            object.renderLayer > static_cast<int>(RenderLayer::UI) ||
            (animationLoaded != 0 && animationLoaded != 1) ||
            (looping != 0 && looping != 1) ||
            (playing != 0 && playing != 1) ||
            !std::isfinite(object.position.x) ||
            !std::isfinite(object.position.y) ||
            !std::isfinite(object.rotation) ||
            !std::isfinite(object.scale.x) ||
            !std::isfinite(object.scale.y) ||
            !std::isfinite(object.animationFPS) ||
            object.animationFPS <= 0.0f ||
            (object.animationLoaded &&
             (object.animationPath.empty() || object.frameWidth <= 0 ||
              object.frameHeight <= 0 || object.frameCount <= 0 ||
              object.currentFrame < 0 || object.currentFrame >= object.frameCount)))
        {
            return false;
        }
    }

    std::string trailingData;
    if (file >> trailingData)
    {
        return false;
    }

    // Reject parent cycles before allocating GPU resources or changing the scene.
    std::vector<unsigned char> visitState(objectCount, 0);
    for (std::size_t start = 0; start < objectCount; ++start)
    {
        if (savedObjects[start].isNull)
        {
            continue;
        }
        std::vector<std::size_t> chain;
        long long current = static_cast<long long>(start);
        while (current >= 0)
        {
            const std::size_t currentIndex = static_cast<std::size_t>(current);
            if (savedObjects[currentIndex].isNull)
            {
                return false;
            }
            if (visitState[currentIndex] == 1)
            {
                return false;
            }
            if (visitState[currentIndex] == 2)
            {
                break;
            }

            visitState[currentIndex] = 1;
            chain.push_back(currentIndex);
            current = savedObjects[currentIndex].parentIndex;
        }
        for (const std::size_t index : chain)
        {
            visitState[index] = 2;
        }
    }

    std::vector<std::unique_ptr<GameObject>> replacement(objectCount);
    for (std::size_t index = 0; index < objectCount; ++index)
    {
        const SavedObject& saved = savedObjects[index];
        if (saved.isNull)
        {
            continue;
        }

        auto object = std::make_unique<GameObject>();
        object->SetName(saved.name);
        object->GetTransform().SetPosition(saved.position);
        object->GetTransform().SetRotation(saved.rotation);
        object->GetTransform().SetScale(saved.scale);
        object->SetRenderLayer(static_cast<RenderLayer>(saved.renderLayer));
        object->SetSortingOrder(saved.sortingOrder);

        if (!saved.spritePath.empty() && !object->SetSprite(saved.spritePath))
        {
            return false;
        }

        AnimationComponent& animation = object->GetAnimation();
        animation.SetFPS(saved.animationFPS);
        animation.SetLooping(saved.looping);
        if (saved.animationLoaded)
        {
            if (!animation.LoadSpriteSheet(
                    saved.animationPath,
                    saved.frameWidth,
                    saved.frameHeight,
                    saved.frameCount))
            {
                return false;
            }
            animation.SetFPS(saved.animationFPS);
            animation.SetLooping(saved.looping);
            animation.SetCurrentFrame(saved.currentFrame);
            if (saved.playing && (saved.looping || saved.currentFrame < saved.frameCount - 1))
            {
                animation.Play();
            }
        }

        replacement[index] = std::move(object);
    }

    for (std::size_t index = 0; index < objectCount; ++index)
    {
        if (replacement[index] == nullptr || savedObjects[index].parentIndex < 0)
        {
            continue;
        }
        GameObject* parent = replacement[static_cast<std::size_t>(savedObjects[index].parentIndex)].get();
        replacement[index]->SetParent(parent);
    }

    ClearSelection();
    selectedObjects.clear();
    selectedObject = nullptr;
    gameObjects.swap(replacement);
    return true;
}
