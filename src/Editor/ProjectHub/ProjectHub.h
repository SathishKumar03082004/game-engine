#pragma once

#include "raylib.h"
#include <filesystem>
#include <string>
#include <vector>

class ProjectHub
{
public:
    // Runs the launcher and returns the selected project root, or an empty path on exit.
    std::filesystem::path Run();

private:
    std::vector<std::filesystem::path> recentProjects;
    std::filesystem::path settingsFile;
    std::string projectName = "My First Game";
    std::string location;
    std::string openPath;
    std::string status;
    bool projectNameFocused = false;
    int recentScroll = 0;

    void LoadRecentProjects();
    void SaveRecentProjects() const;
    void AddRecentProject(const std::filesystem::path& project);
    std::filesystem::path CreateProject();
    std::filesystem::path OpenProject(const std::filesystem::path& project) const;
    void Draw() const;
    static bool IsApex2DProject(const std::filesystem::path& root);
    static std::filesystem::path DefaultProjectsLocation();
};
