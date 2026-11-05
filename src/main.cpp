#include "raylib.h"
#include "Engine/Core/Engine.h"
#include "Editor/ProjectHub/ProjectHub.h"
#include <filesystem>
#include <iostream>
using namespace std;


int main(){
	ProjectHub hub;
	const std::filesystem::path projectRoot = hub.Run();
	if (projectRoot.empty()) return 0;
	std::error_code error;
	std::filesystem::current_path(projectRoot, error);
	if (error) return 1;

	Engine engine;

	engine.Initialize();
	engine.Run();
	engine.Shutdown();
}
