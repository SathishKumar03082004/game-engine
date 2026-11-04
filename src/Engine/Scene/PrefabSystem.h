#pragma once

#include <string>

class GameObject;
class Scene;

class PrefabSystem
{
public:
    static bool Save(const GameObject& root, const std::string& path);
    static GameObject* Instantiate(Scene& scene, const std::string& path);
};
