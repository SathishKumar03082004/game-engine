#include "SceneManager.h"
#include "../Scene/Scene.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>

bool SceneManager::CreateScene(Scene& scene, const std::string& requestedName)
{
    if (requestedName.empty()) return false;
    std::string name = requestedName;
    for (char& character : name)
    {
        const unsigned char value = static_cast<unsigned char>(character);
        if (!std::isalnum(value) && character != '-' && character != '_') character = '_';
    }
    const std::filesystem::path directory("Assets/Scenes");
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) return false;
    std::filesystem::path path = directory / (name + ".scene");
    for (int suffix = 1; std::filesystem::exists(path, error); ++suffix)
    {
        error.clear();
        path = directory / (name + "_" + std::to_string(suffix) + ".scene");
    }
    {
        std::ofstream file(path, std::ios::trunc);
        if (!file) return false;
        file << "{\n  \"format\": \"raylib-scene\",\n  \"version\": 3,\n  \"objects\": []\n}\n";
        if (!file.good()) return false;
    }
    return SwitchScene(scene, path.string());
}

bool SceneManager::SwitchScene(Scene& scene, const std::string& path)
{
    if (path.empty() || !scene.LoadFromFile(path)) return false;
    activeScenePath = std::filesystem::path(path).lexically_normal().generic_string();
    return true;
}

bool SceneManager::SaveActiveScene(const Scene& scene) const
{
    return !activeScenePath.empty() && scene.SaveToFile(activeScenePath);
}

const std::string& SceneManager::GetActiveScenePath() const { return activeScenePath; }

std::vector<std::string> SceneManager::GetScenes() const
{
    std::vector<std::string> scenes;
    std::error_code error;
    for (const std::filesystem::path& directory : {std::filesystem::path("Assets/Scenes"), std::filesystem::path("resources/scenes")})
    {
        error.clear();
        for (std::filesystem::directory_iterator it(directory, error), end; !error && it != end; it.increment(error))
            if (it->is_regular_file(error) && it->path().extension() == ".scene")
                scenes.push_back(it->path().lexically_normal().generic_string());
    }
    std::sort(scenes.begin(), scenes.end());
    return scenes;
}

bool SceneManager::SwitchRelative(Scene& scene, int direction)
{
    const std::vector<std::string> scenes = GetScenes();
    if (scenes.empty()) return false;
    const auto active = std::find(scenes.begin(), scenes.end(), activeScenePath);
    std::size_t index = active == scenes.end() ? 0 : static_cast<std::size_t>(active - scenes.begin());
    const int count = static_cast<int>(scenes.size());
    index = static_cast<std::size_t>((static_cast<int>(index) + (direction < 0 ? count - 1 : 1)) % count);
    return SwitchScene(scene, scenes[index]);
}
