#include "raylib.h"
#include "Engine/Core/Engine.h"
#include <iostream>
using namespace std;


int main(){
	Engine engine;

	engine.Initialize();
	engine.Run();
	engine.Shutdown();
}