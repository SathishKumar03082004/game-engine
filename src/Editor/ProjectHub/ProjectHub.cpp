#include "ProjectHub.h"
#include "../FileDialog.h"

#include "../../Engine/Project/ProjectSettings.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>

namespace
{
    constexpr Color BACKGROUND{28, 30, 36, 255};
    constexpr Color PANEL{38, 41, 49, 255};
    constexpr Color FIELD_COLOR{24, 26, 31, 255};
    constexpr Color ACCENT{83, 155, 255, 255};
    constexpr Color MUTED{158, 165, 178, 255};

    bool Button(Rectangle bounds, const char* label, bool primary = false)
    {
        const Vector2 mouse = GetMousePosition();
        const bool hovered = CheckCollisionPointRec(mouse, bounds);
        DrawRectangleRounded(bounds, 0.12f, 8,
            primary ? (hovered ? Color{104, 172, 255, 255} : ACCENT) :
                      (hovered ? Color{67, 72, 83, 255} : Color{54, 58, 68, 255}));
        const int width = MeasureText(label, 16);
        DrawText(label, static_cast<int>(bounds.x + (bounds.width - width) * 0.5f),
            static_cast<int>(bounds.y + (bounds.height - 16.0f) * 0.5f), 16, RAYWHITE);
        return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    void DisplayPathField(Rectangle bounds, const std::string& value)
    {
        DrawRectangleRounded(bounds, 0.08f, 6, FIELD_COLOR);
        DrawRectangleRoundedLinesEx(bounds, 0.08f, 6, 1.0f, Color{72, 77, 89, 255});
        std::string display = value;
        while (MeasureText(display.c_str(), 13) > static_cast<int>(bounds.width - 24.0f) && display.size() > 8)
            display = "..." + display.substr(display.size() - 28);
        DrawText(display.empty() ? "No folder selected" : display.c_str(),
            static_cast<int>(bounds.x + 12.0f), static_cast<int>(bounds.y + 12.0f), 15,
            value.empty() ? MUTED : RAYWHITE);
    }
}

std::filesystem::path ProjectHub::DefaultProjectsLocation()
{
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile != nullptr) return std::filesystem::path(userProfile) / "Documents" / "Apex2D Projects";
    const char* home = std::getenv("HOME");
    return (home != nullptr ? std::filesystem::path(home) : std::filesystem::current_path()) / "Apex2D Projects";
}

bool ProjectHub::IsApex2DProject(const std::filesystem::path& root)
{
    ProjectSettings settings;
    return settings.Load((root / "resources" / "project.settings").string());
}

void ProjectHub::LoadRecentProjects()
{
    const char* localAppData = std::getenv("LOCALAPPDATA");
    const char* home = std::getenv("HOME");
    const std::filesystem::path configRoot = localAppData != nullptr
        ? std::filesystem::path(localAppData) / "Apex2D"
        : (home != nullptr ? std::filesystem::path(home) / ".config" / "Apex2D"
                           : std::filesystem::current_path() / ".apex2d");
    settingsFile = configRoot / "recent-projects.txt";
    std::error_code error;
    std::filesystem::create_directories(configRoot, error);

    std::ifstream input(settingsFile);
    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty()) continue;
        const std::filesystem::path path = std::filesystem::path(line).lexically_normal();
        if (IsApex2DProject(path) && std::find(recentProjects.begin(), recentProjects.end(), path) == recentProjects.end())
            recentProjects.push_back(path);
    }
}

void ProjectHub::SaveRecentProjects() const
{
    std::error_code error;
    std::filesystem::create_directories(settingsFile.parent_path(), error);
    std::ofstream output(settingsFile, std::ios::trunc);
    for (const auto& project : recentProjects) output << project.string() << '\n';
}

void ProjectHub::AddRecentProject(const std::filesystem::path& project)
{
    const std::filesystem::path normalized = std::filesystem::absolute(project).lexically_normal();
    recentProjects.erase(std::remove(recentProjects.begin(), recentProjects.end(), normalized), recentProjects.end());
    recentProjects.insert(recentProjects.begin(), normalized);
    if (recentProjects.size() > 12) recentProjects.resize(12);
    SaveRecentProjects();
}

std::filesystem::path ProjectHub::CreateProject()
{
    std::string cleanName;
    for (const unsigned char character : projectName)
        if (std::isalnum(character) || character == '-' || character == '_') cleanName.push_back(static_cast<char>(character));
        else if (character == ' ' && !cleanName.empty() && cleanName.back() != '_') cleanName.push_back('_');
    while (!cleanName.empty() && cleanName.back() == '_') cleanName.pop_back();
    if (cleanName.empty()) { status = "Enter a project name using letters or numbers."; return {}; }

    const std::filesystem::path root = std::filesystem::path(location) / cleanName;
    std::error_code error;
    if (std::filesystem::exists(root, error)) { status = "That project folder already exists. Choose another name or location."; return {}; }
    std::filesystem::create_directories(root / "Assets" / "Sprites", error);
    if (!error) std::filesystem::create_directories(root / "Assets" / "Scenes", error);
    if (!error) std::filesystem::create_directories(root / "Assets" / "Prefabs", error);
    if (!error) std::filesystem::create_directories(root / "resources" / "scenes", error);
    if (!error) std::filesystem::create_directories(root / "resources" / "prefabs", error);
    if (error) { status = "Could not create project folders: " + error.message(); return {}; }

    ProjectSettings settings;
    settings.projectName = projectName;
    settings.startupScene = "Assets/Scenes/Main.scene";
    if (!settings.Save((root / "resources" / "project.settings").string()))
    { status = "Could not write project settings."; return {}; }
    std::ofstream scene(root / settings.startupScene, std::ios::trunc);
    if (!scene) { status = "Could not create the starter scene."; return {}; }
    scene << "{\n  \"format\": \"raylib-scene\",\n  \"version\": 3,\n  \"objects\": []\n}\n";
    scene.close();
    if (!scene.good()) { status = "Could not finish writing the starter scene."; return {}; }
    AddRecentProject(root);
    return std::filesystem::absolute(root).lexically_normal();
}

std::filesystem::path ProjectHub::OpenProject(const std::filesystem::path& requested) const
{
    std::error_code error;
    const std::filesystem::path root = std::filesystem::absolute(requested, error).lexically_normal();
    if (error || !IsApex2DProject(root)) return {};
    return root;
}

void ProjectHub::Draw() const
{
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    ClearBackground(BACKGROUND);
    DrawRectangle(0, 0, width, 74, Color{22, 24, 29, 255});
    DrawText("APEX2D", 32, 17, 28, RAYWHITE);
    DrawText("PROJECT HUB", 151, 28, 13, ACCENT);
    DrawText("Create, open and manage your 2D game projects", 32, 86, 16, MUTED);

    const Rectangle createPanel{32, 124, 456, static_cast<float>(height - 164)};
    const Rectangle recentPanel{508, 124, static_cast<float>(width - 540), static_cast<float>(height - 164)};
    DrawRectangleRounded(createPanel, 0.025f, 10, PANEL);
    DrawRectangleRounded(recentPanel, 0.025f, 10, PANEL);
    DrawText("Create a new project", 54, 148, 21, RAYWHITE);
    DrawText("Project name", 54, 196, 14, MUTED);
    DrawRectangleRounded({52, 215, 410, 42}, 0.08f, 6, FIELD_COLOR);
    DrawRectangleRoundedLinesEx({52, 215, 410, 42}, 0.08f, 6, 1.0f, Color{72, 77, 89, 255});
    DrawText(projectName.c_str(), 64, 228, 15, RAYWHITE);
    DrawText("Project location", 54, 272, 14, MUTED);
    DisplayPathField({52, 291, 300, 42}, location);
    Button({360, 291, 102, 42}, "Browse...");
    DrawText("Apex2D creates an Assets folder and a starter scene.", 54, 356, 12, MUTED);
    DrawText("Your Asset Browser will stay inside this project's Assets folder.", 54, 376, 12, MUTED);
    Button({52, 402, 410, 42}, "Create project", true);

    DrawText("Open an existing project", 54, 456, 18, RAYWHITE);
    DrawText("Project folder path", 54, 488, 14, MUTED);
    DisplayPathField({52, 506, 300, 38}, openPath);
    Button({360, 506, 102, 38}, "Browse...");
    DrawText("Choose a project folder here, or reopen a recent project.", 54, 552, 11, MUTED);
    Button({52, 580, 410, 38}, "Open project");

    DrawText("Recent projects", static_cast<int>(recentPanel.x + 24), 148, 21, RAYWHITE);
    if (recentProjects.empty())
        DrawText("Projects you open will appear here.", static_cast<int>(recentPanel.x + 24), 192, 14, MUTED);
    const float rowHeight = 64.0f;
    for (std::size_t index = 0; index < recentProjects.size(); ++index)
    {
        const int row = static_cast<int>(index) - recentScroll;
        const Rectangle rowRect{recentPanel.x + 18, 184.0f + row * rowHeight, recentPanel.width - 36, 54};
        if (rowRect.y + rowRect.height > recentPanel.y + recentPanel.height - 24) continue;
        const bool hovered = CheckCollisionPointRec(GetMousePosition(), rowRect);
        DrawRectangleRounded(rowRect, 0.08f, 6, hovered ? Color{56, 62, 74, 255} : Color{46, 50, 59, 255});
        const std::string name = recentProjects[index].filename().string();
        DrawText(name.c_str(), static_cast<int>(rowRect.x + 14), static_cast<int>(rowRect.y + 8), 15, RAYWHITE);
        const std::string path = recentProjects[index].string();
        DrawText(path.size() > 70 ? TextFormat("...%s", path.substr(path.size() - 67).c_str()) : path.c_str(),
            static_cast<int>(rowRect.x + 14), static_cast<int>(rowRect.y + 31), 10, MUTED);
        DrawText("Open  >", static_cast<int>(rowRect.x + rowRect.width - 66), static_cast<int>(rowRect.y + 18), 11, ACCENT);
    }
    if (!status.empty()) DrawText(status.c_str(), 32, height - 27, 12, Color{255, 190, 120, 255});
    DrawText("Apex2D  |  2D Game Development", width - 270, height - 27, 11, MUTED);
}

std::filesystem::path ProjectHub::Run()
{
    location = DefaultProjectsLocation().string();
    LoadRecentProjects();
    openPath = std::filesystem::current_path().string();
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1100, 700, "Apex2D Hub");
    SetWindowMinSize(900, 620);
    SetTargetFPS(60);
    std::filesystem::path selectedProject;

    while (!WindowShouldClose() && selectedProject.empty())
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();
        const Vector2 mouse = GetMousePosition();
        const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        const bool newClicked = clicked && CheckCollisionPointRec(mouse, {52, 402, 410, 42});
        const bool openClicked = clicked && CheckCollisionPointRec(mouse, {52, 580, 410, 38});
        if (clicked && CheckCollisionPointRec(mouse, {52, 215, 410, 42})) projectNameFocused = true;
        else if (clicked && !CheckCollisionPointRec(mouse, {52, 215, 410, 42})) projectNameFocused = false;
        if (clicked && CheckCollisionPointRec(mouse, {360, 291, 102, 42}))
        {
            const std::string selected = FileDialog::SelectFolder("Choose where Apex2D creates projects", location, GetWindowHandle());
            if (!selected.empty()) location = selected;
        }
        if (clicked && CheckCollisionPointRec(mouse, {360, 506, 102, 38}))
        {
            const std::string selected = FileDialog::SelectFolder("Select an Apex2D project folder", openPath, GetWindowHandle());
            if (!selected.empty()) openPath = selected;
        }

        if (projectNameFocused)
        {
            int character = GetCharPressed();
            while (character > 0)
            {
                if (character >= 32 && character < 127 && projectName.size() < 80)
                    projectName.push_back(static_cast<char>(character));
                character = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !projectName.empty()) projectName.pop_back();
        }
        if (newClicked) selectedProject = CreateProject();
        if (openClicked)
        {
            selectedProject = OpenProject(openPath);
            if (selectedProject.empty()) status = "That folder is not an Apex2D project (resources/project.settings is missing).";
        }

        const Rectangle recentPanel{508, 124, static_cast<float>(width - 540), static_cast<float>(height - 164)};
        const int maxVisible = std::max(1, static_cast<int>((recentPanel.height - 74) / 64.0f));
        const int maxScroll = std::max(0, static_cast<int>(recentProjects.size()) - maxVisible);
        if (CheckCollisionPointRec(GetMousePosition(), recentPanel))
            recentScroll = std::clamp(recentScroll - static_cast<int>(GetMouseWheelMove()), 0, maxScroll);
        if (clicked)
        {
            for (std::size_t index = 0; index < recentProjects.size(); ++index)
            {
                const int row = static_cast<int>(index) - recentScroll;
                const Rectangle rowRect{recentPanel.x + 18, 184.0f + row * 64.0f, recentPanel.width - 36, 54};
                if (CheckCollisionPointRec(mouse, rowRect))
                {
                    selectedProject = OpenProject(recentProjects[index]);
                    if (!selectedProject.empty()) AddRecentProject(selectedProject);
                    break;
                }
            }
        }

        BeginDrawing();
        Draw();
        EndDrawing();
    }
    CloseWindow();
    if (!selectedProject.empty()) AddRecentProject(selectedProject);
    return selectedProject;
}
