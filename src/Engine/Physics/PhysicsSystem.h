#pragma once

#include "raylib.h"
#include <vector>

class Scene;
class BoxCollider2D;
class Rigidbody2D;
class GameObject;
class Tilemap;

struct CollisionEvent
{
    GameObject* first = nullptr;
    GameObject* second = nullptr;
    bool trigger = false;
};

struct TileCollisionEvent
{
    GameObject* object = nullptr;
    int tileX = 0;
    int tileY = 0;
};

struct CollisionPair
{
    BoxCollider2D* first = nullptr;
    BoxCollider2D* second = nullptr;
};

class PhysicsSystem
{
public:
    void Update(Scene& scene);
    void SetGravity(Vector2 value);
    Vector2 GetGravity() const;
    const std::vector<CollisionPair>& GetCollisionPairs() const;
    bool AreOverlapping(const BoxCollider2D& first, const BoxCollider2D& second) const;
    void Clear();

private:
    void ResolveCollisions(const std::vector<BoxCollider2D*>& colliders, const Tilemap& tilemap, int pass);
    void RefreshCollisionPairs(const std::vector<BoxCollider2D*>& colliders);

    std::vector<CollisionPair> collisionPairs;
    std::vector<CollisionPair> previousCollisionPairs;
    Vector2 gravity{0.0f, 980.0f};
};
