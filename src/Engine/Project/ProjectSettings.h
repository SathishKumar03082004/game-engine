#pragma once

#include <string>

class ProjectSettings
{
public:
    std::string projectName = "My 2D Engine";
    int windowWidth = 1280;
    int windowHeight = 720;
    int targetFPS = 60;
    bool windowResizable = true;
    std::string startupScene = "resources/scenes/current.scene";
    std::string buildConfiguration = "release_x64";
    std::string outputDirectory = "build/standalone";

    bool Load(const std::string& path);
    bool Save(const std::string& path) const;
};
