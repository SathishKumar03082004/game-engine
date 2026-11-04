#include "ProjectSettings.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>

bool ProjectSettings::Load(const std::string& path)
{
    std::ifstream file(path);
    if (!file) return false;
    std::string magic;
    int version = 0;
    ProjectSettings loaded;
    int resizable = 1;
    if (!(file >> magic >> version >> std::quoted(loaded.projectName)
               >> loaded.windowWidth >> loaded.windowHeight >> loaded.targetFPS >> resizable
               >> std::quoted(loaded.startupScene) >> std::quoted(loaded.buildConfiguration)
               >> std::quoted(loaded.outputDirectory)) || magic != "RAYLIB_PROJECT" || version != 1 ||
        loaded.projectName.empty() || loaded.windowWidth < 320 || loaded.windowWidth > 7680 ||
        loaded.windowHeight < 240 || loaded.windowHeight > 4320 || loaded.targetFPS < 15 ||
        loaded.targetFPS > 360 || (resizable != 0 && resizable != 1) || loaded.startupScene.empty() ||
        (loaded.buildConfiguration != "release_x64" && loaded.buildConfiguration != "release_x86" &&
         loaded.buildConfiguration != "release_arm64")) return false;
    loaded.windowResizable = resizable != 0;
    *this = std::move(loaded);
    return true;
}

bool ProjectSettings::Save(const std::string& path) const
{
    if (projectName.empty() || windowWidth < 320 || windowHeight < 240 || targetFPS < 15 || startupScene.empty()) return false;
    std::ofstream file(path, std::ios::trunc);
    if (!file) return false;
    file << "RAYLIB_PROJECT 1 " << std::quoted(projectName) << ' ' << windowWidth << ' '
         << windowHeight << ' ' << targetFPS << ' ' << (windowResizable ? 1 : 0) << ' '
         << std::quoted(startupScene) << ' ' << std::quoted(buildConfiguration) << ' '
         << std::quoted(outputDirectory) << '\n';
    return file.good();
}
