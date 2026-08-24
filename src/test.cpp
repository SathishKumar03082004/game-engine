//Button Click Movement
// #include "raylib.h"

// int main()
// {
//     const int screenWidth = 800;
//     const int screenHeight = 800;

//     Rectangle button = { 300, 200, 200, 60 };

//     bool showhello = false;

//     InitWindow(screenWidth, screenHeight, "CPP RayLib Test");

//     SetTargetFPS(60);

//     while (!WindowShouldClose())
//     {
//         Vector2 mouseposition = GetMousePosition();

//         if (CheckCollisionPointRec(mouseposition, button))
//         {
//             if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
//             {
//                 showhello = true;
//             }
//         }

//         BeginDrawing();

//         ClearBackground(RAYWHITE);

//         DrawRectangleRec(button, DARKBLUE);

//         if (CheckCollisionPointRec(mouseposition, button))
//         {
//             DrawRectangleRec(button, YELLOW);
//         }

//         DrawText("Print Hello", 350, 220, 20, WHITE);

//         if (showhello)
//         {
//             DrawText("Hello", 400, 400, 20, BLUE);
//         }

//         EndDrawing();
//     }

//     CloseWindow();

//     return 0;
// }




//Basic Movement

// #include "raylib.h"
// #include <iostream>
// using namespace std;

// int main()
// {
// 	int screenWidth = 800;
// 	int screenHeight = 800;

// 	int textX = 100;
// 	int textY = 100;
// 	int fontSize = 20;

// 	int movement = 50;
// 	int fontIncrease = 10;

// 	InitWindow(screenWidth, screenHeight, "CPP RayLib Test");

// 	while (!WindowShouldClose())
// 	{

// 		if(IsKeyPressed(KEY_A)){
// 			textX-=movement;
// 		}

// 		if(IsKeyPressed(KEY_D)){
// 			textX+=movement;
// 		}

// 		if(IsKeyPressed(KEY_W)){
// 			textY-=movement;
// 		}

// 		if(IsKeyPressed(KEY_S)){
// 			textY+=movement;
// 		}

// 		if(IsKeyPressed(KEY_K)){
// 			fontSize+=fontIncrease;
// 		}

// 		BeginDrawing();

// 		ClearBackground(BLACK);

// 		DrawText("Hello", textX, textY, fontSize, WHITE);

// 		EndDrawing();
// 	}
	
// 	CloseWindow();
// }


//Player Movement

// #include "raylib.h"
// #include <iostream>
// using namespace std;

// int main()
// {
// 	int screenWidth = 800;
// 	int screenHeight = 800;

// 	InitWindow(screenWidth, screenHeight, "CPP RayLib Test");

	
//  	int textX = 100;
//  	int textY = 100;
// 	int playerMovement = 50;

// 	Texture2D player = LoadTexture("resources/hand.png");

// 	while (!WindowShouldClose())
// 	{

// 		if(IsKeyPressed(KEY_A)){
// 			textX-=playerMovement;
// 		}

// 		if(IsKeyPressed(KEY_D)){
// 			textX+=playerMovement;
// 		}

// 		if(IsKeyPressed(KEY_W)){
// 			textY-=playerMovement;
// 		}

// 		if(IsKeyPressed(KEY_S)){
// 			textY+=playerMovement;
// 		}

// 		BeginDrawing();

// 		ClearBackground(BLACK);

// 		DrawTexture(player, textX, textY, WHITE);

// 		EndDrawing();
// 	}
	
// 	CloseWindow();
// }



//Shape Drawning


// #include "raylib.h"
// #include <iostream>
// using namespace std;

// int main()
// {
// 	int screenWidth = 800;
// 	int screenHeight = 800;

// 	InitWindow(screenWidth, screenHeight, "CPP RayLib Test");

// 	while (!WindowShouldClose())
// 	{

// 		// Rectangle rec = {400, 400, 50, 50};

// 		// Vector2 center = {400, 400};

// 		Vector2 top[] = {{100, 100}, {200, 100}};

// 		Vector2 right[] = {{200, 100}, {200, 200}};

// 		Vector2 bottom[] = {{200, 200}, {100, 200}};

// 		Vector2 left[] = {{100,200}, {100, 100}};

// 		BeginDrawing();

// 		ClearBackground(BLACK);

// 		// DrawText("Hello", 100, 100, 20, YELLOW);

// 		// DrawRectangle(300, 300, 50, 50, RED);
// 		// DrawRectangleRounded(rec, 5, 10, BLUE);
// 		// DrawTriangle(Vector2{400, 200,}, Vector2{300, 500}, Vector2{500, 500}, WHITE);
// 		// DrawPoly(center, 6, 50, 0, GREEN);

// 		DrawSplineLinear(top, 2, 3, WHITE);
// 		DrawSplineLinear(right, 2, 3, YELLOW);
// 		DrawSplineLinear(bottom, 2, 3, GREEN);
// 		DrawSplineLinear(left, 2, 3, BLUE);

// 		EndDrawing();
// 	}
	
// 	CloseWindow();
// }




//Mouse Drawing


// #include "raylib.h"
// #include <iostream>
// #include <vector>
// using namespace std;

// int main()
// {
// 	int screenWidth = 800;
// 	int screenHeight = 800;

// 	InitWindow(screenWidth, screenHeight, "CPP RayLib Test");

// 	vector<Vector2> points;

// 	while (!WindowShouldClose())
// 	{
// 		if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
// 			Vector2 mouse = GetMousePosition();
// 			points.push_back(mouse);
// 		}

// 		BeginDrawing();

// 		ClearBackground(BLACK);

// 		for(int i=0;i<points.size();i++){
// 			DrawCircleV(points[i], 5, YELLOW);
// 		}

// 		EndDrawing();
// 	}
	
// 	CloseWindow();
// }




//Player Movement


// #include "raylib.h"
// #include <iostream>
// using namespace std;

// struct Player
// {
// 	Vector2 position;
// 	float speed;
// 	float radius;
// 	Color color;
// };


// int main()
// {
// 	int screenWidth = 800;
// 	int screenHeight = 800;

// 	InitWindow(screenWidth, screenHeight, "CPP RayLib Test");

// 	Player player;

// 	player.position = {400, 300};
// 	player.speed = 50.0f;
// 	player.radius = 20.0f;
// 	player.color = RED;


// 	while (!WindowShouldClose())
// 	{
// 		float deltaTime = GetFrameTime();

// 		if(IsKeyDown(KEY_RIGHT)){
// 			player.position.x += player.speed * deltaTime;
// 		}

// 		if(IsKeyDown(KEY_LEFT)){
// 			player.position.x -= player.speed*deltaTime;
// 		}

// 		if(IsKeyDown(KEY_UP)){
// 			player.position.y -= player.speed*deltaTime;
// 		}

// 		if(IsKeyDown(KEY_DOWN)){
// 			player.position.y += player.speed*deltaTime;
// 		}

// 		BeginDrawing();

// 		ClearBackground(WHITE);

// 		DrawCircleV(player.position, player.radius, player.color);

// 		EndDrawing();
// 	}
	
// 	CloseWindow();
// }



