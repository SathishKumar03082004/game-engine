#include "raylib.h"
#include <iostream>
using namespace std;


int main()
{
	int screenWidth = 800;
	int screenHeight = 800;

	InitWindow(screenWidth, screenHeight, "CPP RayLib Test");

	while (!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(WHITE);
		DrawText("Hello", 100, 100, 20, RED);

		EndDrawing();
	}
	
	CloseWindow();
}