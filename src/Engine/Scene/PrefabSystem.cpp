#include "PrefabSystem.h"

#include "GameObject.h"
#include "Scene.h"
#include "../Rendering/RenderLayer.h"
#include "../Animation/AnimationComponent.h"
#include "../Physics/BoxCollider2D.h"
#include "../Physics/Rigidbody2D.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <vector>

namespace
{
    struct PrefabObject
    {
        long long parent = -1;
        std::string name;
        Vector2 position{};
        float rotation = 0.0f;
        Vector2 scale{1.0f, 1.0f};
        int layer = 0;
        int order = 0;
        std::string sprite;
        Color tint = WHITE;
        std::string activeAnimation;
        std::vector<AnimationClipInfo> animations;
        bool collider = false;
        bool colliderEnabled = true;
        Vector2 colliderSize{100.0f, 100.0f};
        Vector2 colliderOffset{};
        bool trigger = false;
        bool rigidbody = false;
        bool bodyEnabled = true;
        float mass = 1.0f;
        Vector2 velocity{};
        Vector2 acceleration{};
        bool gravity = true;
        bool kinematic = false;
        bool active = true;
    };

    void Collect(GameObject* object, long long parent, std::vector<std::pair<GameObject*, long long>>& output)
    {
        if (object == nullptr) return;
        const long long index = static_cast<long long>(output.size());
        output.push_back({object, parent});
        for (GameObject* child : object->GetChildren()) Collect(child, index, output);
    }

}

bool PrefabSystem::Save(const GameObject& root, const std::string& path)
{
    if (path.empty()) return false;
    std::vector<std::pair<GameObject*, long long>> objects;
    Collect(const_cast<GameObject*>(&root), -1, objects);

    std::error_code error;
    const std::filesystem::path filePath(path);
    if (!filePath.parent_path().empty())
    {
        std::filesystem::create_directories(filePath.parent_path(), error);
        if (error) return false;
    }
    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file) return false;
    file << std::setprecision(std::numeric_limits<float>::max_digits10);
    file << "RAYLIB_PREFAB 2 " << objects.size() << '\n';
    for (const auto& entry : objects)
    {
        GameObject* object = entry.first;
        const TransformComponent& transform = object->GetTransform();
        const bool isPrefabRoot = entry.second < 0;
        const Vector2 position = isPrefabRoot ? object->GetWorldPosition() : transform.GetPosition();
        const Vector2 scale = isPrefabRoot ? object->GetWorldScale() : transform.GetScale();
        const float rotation = isPrefabRoot ? object->GetWorldRotation() : transform.GetRotation();
        const SpriteRenderer& sprite = object->GetSpriteRenderer();
        const Color tint = sprite.GetTint();
        const AnimationComponent* animation = object->GetComponent<AnimationComponent>();
        const std::vector<AnimationClipInfo> clips = animation == nullptr
            ? std::vector<AnimationClipInfo>{} : animation->GetAnimations();
        const BoxCollider2D* collider = object->GetComponent<BoxCollider2D>();
        const Rigidbody2D* body = object->GetComponent<Rigidbody2D>();

        file << "OBJECT " << entry.second << ' ' << std::quoted(object->GetName()) << ' '
             << position.x << ' ' << position.y << ' ' << rotation << ' '
             << scale.x << ' ' << scale.y << ' ' << static_cast<int>(object->GetRenderLayer()) << ' '
             << object->GetSortingOrder() << ' ' << std::quoted(sprite.GetFilePath()) << ' '
             << static_cast<int>(tint.r) << ' ' << static_cast<int>(tint.g) << ' '
             << static_cast<int>(tint.b) << ' ' << static_cast<int>(tint.a) << ' '
             << (object->IsActive() ? 1 : 0) << '\n';

        file << "ANIMATIONS " << clips.size() << ' ' << std::quoted(animation == nullptr
            ? std::string{} : animation->GetCurrentAnimationName()) << '\n';
        for (const AnimationClipInfo& clip : clips)
            file << "ANIMATION " << std::quoted(clip.name) << ' ' << std::quoted(clip.filePath) << ' '
                 << clip.frameWidth << ' ' << clip.frameHeight << ' ' << clip.frameCount << ' '
                 << clip.currentFrame << ' ' << clip.fps << ' ' << (clip.looping ? 1 : 0) << ' '
                 << (clip.playing ? 1 : 0) << '\n';

        file << "COLLIDER " << (collider == nullptr ? 0 : 1) << ' ';
        if (collider != nullptr)
        {
            const Vector2 size = collider->GetSize();
            const Vector2 offset = collider->GetOffset();
            file << (collider->IsEnabled() ? 1 : 0) << ' ' << size.x << ' ' << size.y << ' '
                 << offset.x << ' ' << offset.y << ' ' << (collider->IsTrigger() ? 1 : 0) << ' ';
        }
        else file << "1 100 100 0 0 0 ";

        file << "RIGIDBODY " << (body == nullptr ? 0 : 1) << ' ';
        if (body != nullptr)
        {
            const Vector2 velocity = body->GetVelocity();
            const Vector2 acceleration = body->GetAcceleration();
            file << (body->IsEnabled() ? 1 : 0) << ' ' << body->GetMass() << ' '
                 << velocity.x << ' ' << velocity.y << ' ' << acceleration.x << ' ' << acceleration.y << ' '
                 << (body->IsGravityEnabled() ? 1 : 0) << ' ' << (body->IsKinematic() ? 1 : 0) << '\n';
        }
        else file << "1 1 0 0 0 0 1 0\n";
        if (!file) return false;
    }
    file.flush();
    return file.good();
}

GameObject* PrefabSystem::Instantiate(Scene& scene, const std::string& path)
{
    std::ifstream file(path);
    std::string magic;
    int version = 0;
    std::size_t count = 0;
    if (!(file >> magic >> version >> count) || magic != "RAYLIB_PREFAB" || (version != 1 && version != 2) ||
        count == 0 || count > 10000) return nullptr;

    std::vector<PrefabObject> records(count);
    for (std::size_t index = 0; index < count; ++index)
    {
        PrefabObject& object = records[index];
        std::string marker;
        int layer = 0, tintR = 0, tintG = 0, tintB = 0, tintA = 0;
        if (!(file >> marker >> object.parent >> std::quoted(object.name)
                  >> object.position.x >> object.position.y >> object.rotation
                  >> object.scale.x >> object.scale.y >> layer >> object.order >> std::quoted(object.sprite)
                  >> tintR >> tintG >> tintB >> tintA) || marker != "OBJECT" ||
            object.parent >= static_cast<long long>(index) || object.parent < -1 || object.name.empty() ||
            !std::isfinite(object.position.x) || !std::isfinite(object.position.y) ||
            !std::isfinite(object.rotation) || !std::isfinite(object.scale.x) || !std::isfinite(object.scale.y) ||
            layer < 0 || layer > 4 || tintR < 0 || tintR > 255 || tintG < 0 || tintG > 255 ||
            tintB < 0 || tintB > 255 || tintA < 0 || tintA > 255) return nullptr;
        int active = 1;
        if (version >= 2 && (!(file >> active) || (active != 0 && active != 1))) return nullptr;
        object.active = active != 0;
        object.layer = layer;
        object.tint = {static_cast<unsigned char>(tintR), static_cast<unsigned char>(tintG),
                       static_cast<unsigned char>(tintB), static_cast<unsigned char>(tintA)};

        std::string animationsMarker;
        std::size_t animationCount = 0;
        if (!(file >> animationsMarker >> animationCount >> std::quoted(object.activeAnimation)) ||
            animationsMarker != "ANIMATIONS" || animationCount > 1000) return nullptr;
        object.animations.resize(animationCount);
        for (AnimationClipInfo& clip : object.animations)
        {
            std::string animationMarker;
            int looping = 0, playing = 0;
            if (!(file >> animationMarker >> std::quoted(clip.name) >> std::quoted(clip.filePath)
                      >> clip.frameWidth >> clip.frameHeight >> clip.frameCount >> clip.currentFrame >> clip.fps >> looping >> playing) ||
                animationMarker != "ANIMATION" || clip.name.empty() || clip.filePath.empty() ||
                clip.frameWidth <= 0 || clip.frameHeight <= 0 || clip.frameCount <= 0 ||
                clip.currentFrame < 0 || clip.currentFrame >= clip.frameCount || !std::isfinite(clip.fps) ||
                clip.fps <= 0.0f || (looping != 0 && looping != 1) || (playing != 0 && playing != 1)) return nullptr;
            clip.looping = looping != 0;
            clip.playing = playing != 0;
        }

        std::string colliderMarker;
        int hasCollider = 0;
        if (!(file >> colliderMarker >> hasCollider) || colliderMarker != "COLLIDER" ||
            (hasCollider != 0 && hasCollider != 1)) return nullptr;
        object.collider = hasCollider != 0;
        int colliderEnabled = 0, trigger = 0;
        if (!(file >> colliderEnabled >> object.colliderSize.x >> object.colliderSize.y
                  >> object.colliderOffset.x >> object.colliderOffset.y >> trigger) ||
            (colliderEnabled != 0 && colliderEnabled != 1) || (trigger != 0 && trigger != 1) ||
            !std::isfinite(object.colliderSize.x) || !std::isfinite(object.colliderSize.y) ||
            !std::isfinite(object.colliderOffset.x) || !std::isfinite(object.colliderOffset.y)) return nullptr;
        if (object.collider)
        {
            object.colliderEnabled = colliderEnabled != 0;
            object.trigger = trigger != 0;
        }

        std::string bodyMarker;
        int hasBody = 0;
        if (!(file >> bodyMarker >> hasBody) || bodyMarker != "RIGIDBODY" || (hasBody != 0 && hasBody != 1)) return nullptr;
        object.rigidbody = hasBody != 0;
        int bodyEnabled = 0, gravity = 0, kinematic = 0;
        if (!(file >> bodyEnabled >> object.mass >> object.velocity.x >> object.velocity.y
                  >> object.acceleration.x >> object.acceleration.y >> gravity >> kinematic) ||
            (bodyEnabled != 0 && bodyEnabled != 1) || (gravity != 0 && gravity != 1) ||
            (kinematic != 0 && kinematic != 1) || !std::isfinite(object.mass) || object.mass <= 0.0f ||
            !std::isfinite(object.velocity.x) || !std::isfinite(object.velocity.y) ||
            !std::isfinite(object.acceleration.x) || !std::isfinite(object.acceleration.y)) return nullptr;
        object.bodyEnabled = bodyEnabled != 0;
        object.gravity = gravity != 0;
        object.kinematic = kinematic != 0;
    }

    std::vector<GameObject*> created;
    created.reserve(count);
    for (std::size_t index = 0; index < count; ++index)
    {
        const PrefabObject& saved = records[index];
        GameObject* object = scene.CreateGameObject(saved.name);
        if (object == nullptr) break;
        created.push_back(object);
        object->GetTransform().SetPosition(saved.position);
        object->GetTransform().SetRotation(saved.rotation);
        object->GetTransform().SetScale(saved.scale);
        object->SetRenderLayer(static_cast<RenderLayer>(saved.layer));
        object->SetSortingOrder(saved.order);
        object->SetActive(saved.active);
        if (!saved.sprite.empty() && !object->SetSprite(saved.sprite)) break;
        object->GetSpriteRenderer().SetTint(saved.tint);

        AnimationComponent& animation = object->GetAnimation();
        bool animationsLoaded = true;
        for (const AnimationClipInfo& clip : saved.animations)
        {
            if (!animation.AddAnimation(clip.name, clip.filePath, clip.frameWidth, clip.frameHeight,
                    clip.frameCount, clip.fps, clip.looping))
            {
                animationsLoaded = false;
                break;
            }
        }
        if (!animationsLoaded) break;
        if (!saved.activeAnimation.empty() && animation.Play(saved.activeAnimation))
        {
            const auto active = std::find_if(saved.animations.begin(), saved.animations.end(),
                [&saved](const AnimationClipInfo& clip) { return clip.name == saved.activeAnimation; });
            if (active != saved.animations.end())
            {
                animation.SetCurrentFrame(active->currentFrame);
                animation.SetFPS(active->fps);
                animation.SetLooping(active->looping);
                if (!active->playing) animation.Pause();
            }
        }

        if (saved.collider)
        {
            BoxCollider2D& collider = object->AddComponent<BoxCollider2D>(saved.colliderSize, saved.colliderOffset);
            collider.SetTrigger(saved.trigger);
            collider.SetEnabled(saved.colliderEnabled);
        }
        if (saved.rigidbody)
        {
            Rigidbody2D& body = object->AddComponent<Rigidbody2D>();
            body.SetMass(saved.mass);
            body.SetVelocity(saved.velocity);
            body.SetAcceleration(saved.acceleration);
            body.SetGravityEnabled(saved.gravity);
            body.SetKinematic(saved.kinematic);
            body.SetEnabled(saved.bodyEnabled);
        }
        if (saved.parent >= 0) object->SetParent(created[static_cast<std::size_t>(saved.parent)]);
    }

    if (created.size() != count)
    {
        for (auto it = created.rbegin(); it != created.rend(); ++it) scene.DestroyGameObject(*it);
        return nullptr;
    }
    scene.SelectObject(created.front());
    return created.front();
}
