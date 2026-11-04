#pragma once

#include <string>
#include <vector>

class Scene;

class SceneManager
{
public:
    bool CreateScene(Scene& scene, const std::string& name);
    bool SwitchScene(Scene& scene, const std::string& path);
    bool SaveActiveScene(const Scene& scene) const;
    const std::string& GetActiveScenePath() const;
    std::vector<std::string> GetScenes() const;
    bool SwitchRelative(Scene& scene, int direction);

private:
    std::string activeScenePath;
};
