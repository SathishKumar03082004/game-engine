#pragma once

#include <string>

class ProjectSettings;

class BuildConfiguration
{
public:
    static bool GenerateWindowsBuildScript(const ProjectSettings& settings,
        const std::string& scriptPath = "build_standalone.bat");
};
