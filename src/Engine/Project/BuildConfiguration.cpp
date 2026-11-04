#include "BuildConfiguration.h"
#include "ProjectSettings.h"

#include <algorithm>
#include <cctype>
#include <fstream>

bool BuildConfiguration::GenerateWindowsBuildScript(const ProjectSettings& settings,
    const std::string& scriptPath)
{
    if (scriptPath.empty() || settings.outputDirectory.empty()) return false;
    const bool safeOutputPath = std::all_of(settings.outputDirectory.begin(), settings.outputDirectory.end(),
        [](unsigned char character)
        {
            return std::isalnum(character) || character == '/' || character == '\\' ||
                   character == '_' || character == '-' || character == '.' || character == ' ';
        });
    if (!safeOutputPath || settings.outputDirectory.find("..") != std::string::npos ||
        settings.outputDirectory == "." || settings.outputDirectory == "resources") return false;
    const bool x64 = settings.buildConfiguration == "release_x64";
    const bool x86 = settings.buildConfiguration == "release_x86";
    const bool arm64 = settings.buildConfiguration == "release_arm64";
    if (!x64 && !x86 && !arm64) return false;

    std::ofstream file(scriptPath, std::ios::trunc);
    if (!file) return false;
    file << "@echo off\r\nsetlocal\r\ncd /d \"%~dp0\"\r\n"
         << "where mingw32-make >nul 2>nul\r\nif errorlevel 1 (echo mingw32-make was not found in PATH.& exit /b 1)\r\n"
         << "mingw32-make config=" << settings.buildConfiguration << " game-engine\r\n"
         << "if errorlevel 1 exit /b %errorlevel%\r\n"
         << "if not exist \"" << settings.outputDirectory << "\" mkdir \"" << settings.outputDirectory << "\"\r\n"
         << "copy /Y \"bin\\Release\\game-engine.exe\" \"" << settings.outputDirectory << "\\game-engine.exe\"\r\n"
         << "if errorlevel 1 exit /b %errorlevel%\r\n"
         << "xcopy /E /I /Y \"resources\" \"" << settings.outputDirectory << "\\resources\"\r\n"
         << "if errorlevel 1 exit /b %errorlevel%\r\n"
         << "echo Standalone build is ready in " << settings.outputDirectory << "\r\nendlocal\r\n";
    return file.good();
}
