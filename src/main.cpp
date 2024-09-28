#include "raylib.h"

int main(void)
{
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "Trainering");

	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(RAYWHITE);

		auto t = "Hello, Window!";
		auto a = MeasureText(t, 20);
		DrawText(t, (screenWidth  - a) / 2, 200, 20, LIGHTGRAY);

		EndDrawing();
	}
	CloseWindow();

	return 0;
}