#include "raylib.h"
#include "Engine/Core/Engine.h"
#include <iostream>
using namespace std;


int main(){
	Engine engine;

	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);

	engine.Initialize();
	engine.Run();
	engine.Shutdown();
}