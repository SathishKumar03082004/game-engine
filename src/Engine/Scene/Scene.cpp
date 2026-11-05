#include "Scene.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <variant>
#include <unordered_map>
#include <utility>

namespace
{
    struct JsonValue
    {
        using Array = std::vector<JsonValue>;
        using Object = std::map<std::string, JsonValue>;
        std::variant<std::monostate, bool, double, std::string, Array, Object> value;
    };

    class JsonParser
    {
    public:
        explicit JsonParser(const std::string& source) : text(source) {}

        bool Parse(JsonValue& result)
        {
            position = 0;
            if (!ParseValue(result, 0))
            {
                return false;
            }
            SkipWhitespace();
            return position == text.size();
        }

    private:
        const std::string& text;
        std::size_t position = 0;

        void SkipWhitespace()
        {
            while (position < text.size() &&
                   (text[position] == ' ' || text[position] == '\t' ||
                    text[position] == '\r' || text[position] == '\n'))
            {
                ++position;
            }
        }

        bool Consume(char expected)
        {
            SkipWhitespace();
            if (position >= text.size() || text[position] != expected)
            {
                return false;
            }
            ++position;
            return true;
        }

        static void AppendCodePoint(std::string& output, unsigned int codePoint)
        {
            if (codePoint <= 0x7f)
            {
                output.push_back(static_cast<char>(codePoint));
            }
            else if (codePoint <= 0x7ff)
            {
                output.push_back(static_cast<char>(0xc0 | (codePoint >> 6)));
                output.push_back(static_cast<char>(0x80 | (codePoint & 0x3f)));
            }
            else if (codePoint <= 0xffff)
            {
                output.push_back(static_cast<char>(0xe0 | (codePoint >> 12)));
                output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f)));
                output.push_back(static_cast<char>(0x80 | (codePoint & 0x3f)));
            }
            else
            {
                output.push_back(static_cast<char>(0xf0 | (codePoint >> 18)));
                output.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3f)));
                output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f)));
                output.push_back(static_cast<char>(0x80 | (codePoint & 0x3f)));
            }
        }

        bool ParseString(std::string& output)
        {
            if (position >= text.size() || text[position++] != '"')
            {
                return false;
            }
            output.clear();
            while (position < text.size())
            {
                const unsigned char character = static_cast<unsigned char>(text[position++]);
                if (character == '"')
                {
                    return true;
                }
                if (character < 0x20)
                {
                    return false;
                }
                if (character != '\\')
                {
                    output.push_back(static_cast<char>(character));
                    continue;
                }
                if (position >= text.size())
                {
                    return false;
                }
                const char escaped = text[position++];
                switch (escaped)
                {
                    case '"': output.push_back('"'); break;
                    case '\\': output.push_back('\\'); break;
                    case '/': output.push_back('/'); break;
                    case 'b': output.push_back('\b'); break;
                    case 'f': output.push_back('\f'); break;
                    case 'n': output.push_back('\n'); break;
                    case 'r': output.push_back('\r'); break;
                    case 't': output.push_back('\t'); break;
                    case 'u':
                    {
                        if (position + 4 > text.size()) return false;
                        unsigned int codePoint = 0;
                        for (int i = 0; i < 4; ++i)
                        {
                            const char digit = text[position++];
                            codePoint <<= 4;
                            if (digit >= '0' && digit <= '9') codePoint |= digit - '0';
                            else if (digit >= 'a' && digit <= 'f') codePoint |= digit - 'a' + 10;
                            else if (digit >= 'A' && digit <= 'F') codePoint |= digit - 'A' + 10;
                            else return false;
                        }
                        if (codePoint >= 0xd800 && codePoint <= 0xdbff)
                        {
                            if (position + 6 > text.size() || text[position] != '\\' || text[position + 1] != 'u') return false;
                            position += 2;
                            unsigned int low = 0;
                            for (int i = 0; i < 4; ++i)
                            {
                                const char digit = text[position++];
                                low <<= 4;
                                if (digit >= '0' && digit <= '9') low |= digit - '0';
                                else if (digit >= 'a' && digit <= 'f') low |= digit - 'a' + 10;
                                else if (digit >= 'A' && digit <= 'F') low |= digit - 'A' + 10;
                                else return false;
                            }
                            if (low < 0xdc00 || low > 0xdfff) return false;
                            codePoint = 0x10000 + ((codePoint - 0xd800) << 10) + (low - 0xdc00);
                        }
                        else if (codePoint >= 0xdc00 && codePoint <= 0xdfff)
                        {
                            return false;
                        }
                        AppendCodePoint(output, codePoint);
                        break;
                    }
                    default: return false;
                }
            }
            return false;
        }

        bool ParseValue(JsonValue& result, int depth)
        {
            if (depth > 64)
            {
                return false;
            }
            SkipWhitespace();
            if (position >= text.size()) return false;
            const char character = text[position];

            if (character == '"')
            {
                std::string stringValue;
                if (!ParseString(stringValue)) return false;
                result.value = std::move(stringValue);
                return true;
            }
            if (character == '{')
            {
                ++position;
                JsonValue::Object object;
                SkipWhitespace();
                if (Consume('}')) { result.value = std::move(object); return true; }
                while (true)
                {
                    SkipWhitespace();
                    std::string key;
                    if (!ParseString(key) || !Consume(':')) return false;
                    JsonValue member;
                    if (!ParseValue(member, depth + 1) || !object.emplace(std::move(key), std::move(member)).second) return false;
                    if (Consume('}')) break;
                    if (!Consume(',')) return false;
                }
                result.value = std::move(object);
                return true;
            }
            if (character == '[')
            {
                ++position;
                JsonValue::Array array;
                SkipWhitespace();
                if (Consume(']')) { result.value = std::move(array); return true; }
                while (true)
                {
                    JsonValue item;
                    if (!ParseValue(item, depth + 1)) return false;
                    array.push_back(std::move(item));
                    if (Consume(']')) break;
                    if (!Consume(',')) return false;
                }
                result.value = std::move(array);
                return true;
            }
            if (text.compare(position, 4, "true") == 0) { position += 4; result.value = true; return true; }
            if (text.compare(position, 5, "false") == 0) { position += 5; result.value = false; return true; }
            if (text.compare(position, 4, "null") == 0) { position += 4; result.value = std::monostate{}; return true; }

            const std::size_t start = position;
            if (text[position] == '-') ++position;
            if (position >= text.size()) return false;
            if (text[position] == '0') ++position;
            else
            {
                if (text[position] < '1' || text[position] > '9') return false;
                while (position < text.size() && text[position] >= '0' && text[position] <= '9') ++position;
            }
            if (position < text.size() && text[position] == '.')
            {
                ++position;
                const std::size_t fractionStart = position;
                while (position < text.size() && text[position] >= '0' && text[position] <= '9') ++position;
                if (fractionStart == position) return false;
            }
            if (position < text.size() && (text[position] == 'e' || text[position] == 'E'))
            {
                ++position;
                if (position < text.size() && (text[position] == '+' || text[position] == '-')) ++position;
                const std::size_t exponentStart = position;
                while (position < text.size() && text[position] >= '0' && text[position] <= '9') ++position;
                if (exponentStart == position) return false;
            }
            try
            {
                const double number = std::stod(text.substr(start, position - start));
                if (!std::isfinite(number)) return false;
                result.value = number;
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
    };

    std::string JsonEscape(const std::string& value)
    {
        static const char hex[] = "0123456789abcdef";
        std::string escaped;
        escaped.push_back('"');
        for (unsigned char character : value)
        {
            switch (character)
            {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\b': escaped += "\\b"; break;
                case '\f': escaped += "\\f"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default:
                    if (character < 0x20)
                    {
                        escaped += "\\u00";
                        escaped.push_back(hex[character >> 4]);
                        escaped.push_back(hex[character & 0xf]);
                    }
                    else escaped.push_back(static_cast<char>(character));
            }
        }
        escaped.push_back('"');
        return escaped;
    }

    const JsonValue* JsonMember(const JsonValue& value, const std::string& key)
    {
        const auto* object = std::get_if<JsonValue::Object>(&value.value);
        if (object == nullptr) return nullptr;
        const auto iterator = object->find(key);
        return iterator == object->end() ? nullptr : &iterator->second;
    }

    bool JsonNumber(const JsonValue* value, double& result)
    {
        const auto* number = value == nullptr ? nullptr : std::get_if<double>(&value->value);
        if (number == nullptr || !std::isfinite(*number)) return false;
        result = *number;
        return true;
    }

    bool JsonInteger(const JsonValue* value, long long& result)
    {
        double number = 0.0;
        if (!JsonNumber(value, number) || std::floor(number) != number ||
            number < -9223372036854775808.0 || number >= 9223372036854775808.0) return false;
        result = static_cast<long long>(number);
        return true;
    }

    bool JsonString(const JsonValue* value, std::string& result)
    {
        const auto* string = value == nullptr ? nullptr : std::get_if<std::string>(&value->value);
        if (string == nullptr) return false;
        result = *string;
        return true;
    }

    bool JsonBoolean(const JsonValue* value, bool& result)
    {
        const auto* boolean = value == nullptr ? nullptr : std::get_if<bool>(&value->value);
        if (boolean == nullptr) return false;
        result = *boolean;
        return true;
    }
}

Scene::Scene()
    : selectedObject(nullptr),
      gridSize(64),
      gridExtent(15000),
      gridSnappingEnabled(true)
{
}

void Scene::Clear()
{
    ClearSelection();
    gameObjects.clear();
    tilemap.Clear();
    selectionDragStartPositions.clear();
}

bool Scene::CloneTo(Scene& destination) const
{
    destination.Clear();
    destination.gridSize = gridSize;
    destination.gridExtent = gridExtent;
    destination.gridSnappingEnabled = gridSnappingEnabled;
    if (!tilemap.CloneTo(destination.tilemap)) return false;

    std::unordered_map<const GameObject*, GameObject*> copies;
    copies.reserve(gameObjects.size());
    for (const auto& object : gameObjects)
    {
        if (!object) continue;
        GameObject* copy = destination.CreateGameObject(object->GetName());
        if (copy == nullptr) { destination.Clear(); return false; }
        copy->CopyForRuntimeFrom(*object);
        copies.emplace(object.get(), copy);
    }
    for (const auto& object : gameObjects)
    {
        if (!object || object->GetParent() == nullptr) continue;
        auto copy = copies.find(object.get());
        auto parent = copies.find(object->GetParent());
        if (copy != copies.end() && parent != copies.end()) copy->second->SetParent(parent->second);
    }
    return true;
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
    physicsSystem.Update(*this);
}

PhysicsSystem& Scene::GetPhysicsSystem() { return physicsSystem; }
const PhysicsSystem& Scene::GetPhysicsSystem() const { return physicsSystem; }
Tilemap& Scene::GetTilemap() { return tilemap; }
const Tilemap& Scene::GetTilemap() const { return tilemap; }
const RendererStats& Scene::GetRendererStats() const { return renderer.GetStats(); }


void Scene::Draw()
{
    renderer.BeginFrame();
    DrawGrid();
    tilemap.DrawLayer("Background");
    tilemap.DrawLayer("Ground");

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

    bool objectTilesDrawn = false;
    for (GameObject* object : sortedObjects)
    {
        if (!objectTilesDrawn && static_cast<int>(object->GetRenderLayer()) >= static_cast<int>(RenderLayer::Characters))
        {
            tilemap.DrawLayer("Objects");
            objectTilesDrawn = true;
        }
        object->SubmitRender(renderer);
    }
    renderer.Flush();
    if (!objectTilesDrawn) tilemap.DrawLayer("Objects");
    tilemap.DrawLayer("Foreground");
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
    GameObject* clickedObject = GetObjectAt(worldPosition);
    if (clickedObject == nullptr)
    {
        if (!ctrlPressed) ClearSelection();
        return;
    }

    SelectObject(clickedObject, ctrlPressed);
}

GameObject* Scene::GetObjectAt(Vector2 worldPosition) const
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

    for (auto it = sortedObjects.rbegin();
         it != sortedObjects.rend();
         ++it)
    {
        if ((*it)->ContainsPoint(worldPosition))
        {
            return *it;
        }
    }

    return nullptr;
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

const std::vector<std::unique_ptr<GameObject>>& Scene::GetGameObjects() const
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
    if (selectionDragStartPositions.empty()) BeginSelectionDrag(worldPosition);
    DragSelection(worldPosition);
}

void Scene::BeginSelectionDrag(Vector2 worldPosition)
{
    selectionDragMouseStart = worldPosition;
    selectionDragStartPositions.clear();
    for (GameObject* object : selectedObjects)
        if (object != nullptr) selectionDragStartPositions.push_back({object, object->GetWorldPosition()});
}

void Scene::DragSelection(Vector2 worldPosition)
{
    if (selectionDragStartPositions.empty()) return;
    Vector2 delta{worldPosition.x - selectionDragMouseStart.x,
                  worldPosition.y - selectionDragMouseStart.y};
    if (gridSnappingEnabled)
    {
        const Vector2 primaryTarget{selectionDragStartPositions.front().second.x + delta.x,
                                     selectionDragStartPositions.front().second.y + delta.y};
        const Vector2 snappedTarget = SnapToGrid(primaryTarget);
        delta = {snappedTarget.x - selectionDragStartPositions.front().second.x,
                 snappedTarget.y - selectionDragStartPositions.front().second.y};
    }
    for (const auto& entry : selectionDragStartPositions)
    {
        GameObject* object = entry.first;
        if (object == nullptr || !ContainsGameObject(object)) continue;
        const Vector2 worldPositionTarget{entry.second.x + delta.x, entry.second.y + delta.y};
        GameObject* parent = object->GetParent();
        object->GetTransform().SetPosition(parent == nullptr ? worldPositionTarget : parent->InverseTransformPoint(worldPositionTarget));
    }
}

void Scene::SetGridSize(int size)
{
    gridSize = std::clamp(size, 8, 256);
}

int Scene::GetGridSize() const { return gridSize; }
void Scene::SetGridSnappingEnabled(bool enabled) { gridSnappingEnabled = enabled; }
bool Scene::IsGridSnappingEnabled() const { return gridSnappingEnabled; }

Vector2 Scene::SnapToGrid(Vector2 worldPosition) const
{
    const float step = static_cast<float>(gridSize);
    return {std::round(worldPosition.x / step) * step,
            std::round(worldPosition.y / step) * step};
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

    file << "{\n  \"format\": \"raylib-scene\",\n  \"version\": 2,\n  \"objects\": [\n";
    file << std::setprecision(std::numeric_limits<float>::max_digits10);

    for (std::size_t index = 0; index < gameObjects.size(); ++index)
    {
        GameObject* object = gameObjects[index].get();
        if (object == nullptr)
        {
            file << "    null";
        }
        else
        {
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

            file << "    {\"name\": " << JsonEscape(object->GetName())
                 << ", \"active\": " << (object->IsActive() ? "true" : "false")
                 << ", \"parent\": " << parentIndex
                 << ", \"position\": [" << position.x << ", " << position.y << ']'
                 << ", \"rotation\": " << transform.GetRotation()
                 << ", \"scale\": [" << scale.x << ", " << scale.y << ']'
                 << ", \"renderLayer\": " << static_cast<int>(object->GetRenderLayer())
                 << ", \"sortingOrder\": " << object->GetSortingOrder()
                 << ", \"sprite\": " << JsonEscape(sprite.GetFilePath())
                 << ", \"animation\": {\"loaded\": " << (animation.IsLoaded() ? "true" : "false")
                 << ", \"path\": " << JsonEscape(animation.GetFilePath())
                 << ", \"frameWidth\": " << animation.GetFrameWidth()
                 << ", \"frameHeight\": " << animation.GetFrameHeight()
                 << ", \"frameCount\": " << animation.GetFrameCount()
                 << ", \"currentFrame\": " << animation.GetCurrentFrame()
                 << ", \"fps\": " << animation.GetFPS()
                 << ", \"looping\": " << (animation.IsLooping() ? "true" : "false")
                 << ", \"playing\": " << (animation.IsPlaying() ? "true" : "false")
                 << "}}";
        }

        file << (index + 1 < gameObjects.size() ? ",\n" : "\n");
        if (!file) return false;
    }

    file << "  ]\n}\n";
    file.flush();
    return file.good() && tilemap.Save(path + ".tiles");
}

bool Scene::LoadFromFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    char firstCharacter = '\0';
    while (file.get(firstCharacter) && std::isspace(static_cast<unsigned char>(firstCharacter))) {}
    if (!file) return false;
    const bool sceneLoaded = firstCharacter == '{' ? LoadJsonFromFile(path) : LoadLegacyFromFile(path);
    if (!sceneLoaded) return false;
    const std::string tilemapPath = path + ".tiles";
    std::error_code error;
    if (std::filesystem::exists(tilemapPath, error) && !error) return tilemap.Load(tilemapPath);
    tilemap.Clear();
    return true;
}

bool Scene::LoadJsonFromFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    const std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (source.size() > 32 * 1024 * 1024) return false;

    JsonValue document;
    if (!JsonParser(source).Parse(document)) return false;

    std::string format;
    long long version = 0;
    const JsonValue* objectsValue = JsonMember(document, "objects");
    const auto* objects = objectsValue == nullptr ? nullptr : std::get_if<JsonValue::Array>(&objectsValue->value);
    if (!JsonString(JsonMember(document, "format"), format) || format != "raylib-scene" ||
        !JsonInteger(JsonMember(document, "version"), version) || version != 2 ||
        objects == nullptr || objects->size() > 100000)
    {
        return false;
    }

    struct SavedObject
    {
        bool isNull = false;
        bool active = true;
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

    auto readFloat = [](const JsonValue* value, float& output)
    {
        double number = 0.0;
        if (!JsonNumber(value, number) ||
            number < -std::numeric_limits<float>::max() ||
            number > std::numeric_limits<float>::max()) return false;
        output = static_cast<float>(number);
        return true;
    };
    auto readInt = [](const JsonValue* value, int& output)
    {
        long long number = 0;
        if (!JsonInteger(value, number) || number < std::numeric_limits<int>::min() ||
            number > std::numeric_limits<int>::max()) return false;
        output = static_cast<int>(number);
        return true;
    };

    std::vector<SavedObject> savedObjects(objects->size());
    for (std::size_t index = 0; index < objects->size(); ++index)
    {
        const JsonValue& entry = (*objects)[index];
        if (std::holds_alternative<std::monostate>(entry.value))
        {
            savedObjects[index].isNull = true;
            continue;
        }

        SavedObject& saved = savedObjects[index];
        const JsonValue* positionValue = JsonMember(entry, "position");
        const JsonValue* scaleValue = JsonMember(entry, "scale");
        const JsonValue* animationValue = JsonMember(entry, "animation");
        const auto* position = positionValue == nullptr ? nullptr : std::get_if<JsonValue::Array>(&positionValue->value);
        const auto* scale = scaleValue == nullptr ? nullptr : std::get_if<JsonValue::Array>(&scaleValue->value);
        bool animationLoaded = false;

        if (position == nullptr || position->size() != 2 || scale == nullptr || scale->size() != 2 ||
            !JsonInteger(JsonMember(entry, "parent"), saved.parentIndex) ||
            !JsonString(JsonMember(entry, "name"), saved.name) ||
            !readFloat(&(*position)[0], saved.position.x) || !readFloat(&(*position)[1], saved.position.y) ||
            !readFloat(JsonMember(entry, "rotation"), saved.rotation) ||
            !readFloat(&(*scale)[0], saved.scale.x) || !readFloat(&(*scale)[1], saved.scale.y) ||
            !readInt(JsonMember(entry, "renderLayer"), saved.renderLayer) ||
            !readInt(JsonMember(entry, "sortingOrder"), saved.sortingOrder) ||
            !JsonString(JsonMember(entry, "sprite"), saved.spritePath) ||
            animationValue == nullptr ||
            !JsonBoolean(JsonMember(*animationValue, "loaded"), animationLoaded) ||
            !JsonString(JsonMember(*animationValue, "path"), saved.animationPath) ||
            !readInt(JsonMember(*animationValue, "frameWidth"), saved.frameWidth) ||
            !readInt(JsonMember(*animationValue, "frameHeight"), saved.frameHeight) ||
            !readInt(JsonMember(*animationValue, "frameCount"), saved.frameCount) ||
            !readInt(JsonMember(*animationValue, "currentFrame"), saved.currentFrame) ||
            !readFloat(JsonMember(*animationValue, "fps"), saved.animationFPS) ||
            !JsonBoolean(JsonMember(*animationValue, "looping"), saved.looping) ||
            !JsonBoolean(JsonMember(*animationValue, "playing"), saved.playing))
        {
            return false;
        }
        if (const JsonValue* activeValue = JsonMember(entry, "active"); activeValue != nullptr && !JsonBoolean(activeValue, saved.active))
            return false;
        saved.animationLoaded = animationLoaded;

        if (saved.parentIndex < -1 || saved.parentIndex >= static_cast<long long>(objects->size()) ||
            saved.parentIndex == static_cast<long long>(index) ||
            saved.renderLayer < static_cast<int>(RenderLayer::Background) ||
            saved.renderLayer > static_cast<int>(RenderLayer::UI) ||
            saved.animationFPS <= 0.0f ||
            (saved.animationLoaded && (saved.animationPath.empty() || saved.frameWidth <= 0 ||
             saved.frameHeight <= 0 || saved.frameCount <= 0 || saved.currentFrame < 0 ||
             saved.currentFrame >= saved.frameCount)))
        {
            return false;
        }
    }

    std::vector<unsigned char> visitState(savedObjects.size(), 0);
    for (std::size_t start = 0; start < savedObjects.size(); ++start)
    {
        if (savedObjects[start].isNull) continue;
        std::vector<std::size_t> chain;
        long long current = static_cast<long long>(start);
        while (current >= 0)
        {
            const std::size_t currentIndex = static_cast<std::size_t>(current);
            if (savedObjects[currentIndex].isNull || visitState[currentIndex] == 1) return false;
            if (visitState[currentIndex] == 2) break;
            visitState[currentIndex] = 1;
            chain.push_back(currentIndex);
            current = savedObjects[currentIndex].parentIndex;
        }
        for (const std::size_t index : chain) visitState[index] = 2;
    }

    std::vector<std::unique_ptr<GameObject>> replacement(savedObjects.size());
    for (std::size_t index = 0; index < savedObjects.size(); ++index)
    {
        const SavedObject& saved = savedObjects[index];
        if (saved.isNull) continue;

        auto object = std::make_unique<GameObject>();
        object->SetName(saved.name);
        object->SetActive(saved.active);
        object->GetTransform().SetPosition(saved.position);
        object->GetTransform().SetRotation(saved.rotation);
        object->GetTransform().SetScale(saved.scale);
        object->SetRenderLayer(static_cast<RenderLayer>(saved.renderLayer));
        object->SetSortingOrder(saved.sortingOrder);
        if (!saved.spritePath.empty() && !object->SetSprite(saved.spritePath)) return false;

        AnimationComponent& animation = object->GetAnimation();
        animation.SetFPS(saved.animationFPS);
        animation.SetLooping(saved.looping);
        if (saved.animationLoaded)
        {
            if (!animation.LoadSpriteSheet(saved.animationPath, saved.frameWidth,
                saved.frameHeight, saved.frameCount)) return false;
            animation.SetFPS(saved.animationFPS);
            animation.SetLooping(saved.looping);
            animation.SetCurrentFrame(saved.currentFrame);
            if (saved.playing && (saved.looping || saved.currentFrame < saved.frameCount - 1)) animation.Play();
        }
        replacement[index] = std::move(object);
    }

    for (std::size_t index = 0; index < savedObjects.size(); ++index)
    {
        if (replacement[index] == nullptr || savedObjects[index].parentIndex < 0) continue;
        replacement[index]->SetParent(replacement[static_cast<std::size_t>(savedObjects[index].parentIndex)].get());
    }

    ClearSelection();
    selectedObjects.clear();
    selectedObject = nullptr;
    gameObjects.swap(replacement);
    return true;
}

bool Scene::LoadLegacyFromFile(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }

    struct SavedObject
    {
        bool isNull = false;
        bool active = true;
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
        object->SetActive(saved.active);
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
