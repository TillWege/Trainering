#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

int main(void)
{
	const int screenWidth = 1280;
	const int screenHeight = 720;

	InitWindow(screenWidth, screenHeight, "Trainering");
	rlImGuiSetup(true);
	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		BeginDrawing();
		rlImGuiBegin();

		ClearBackground(RAYWHITE);

		auto t = "Hello, Window!";
		auto a = MeasureText(t, 20);
		DrawText(t, (screenWidth  - a) / 2, (screenHeight - 20) / 2, 20, LIGHTGRAY);

		ImGui::ShowDemoWindow();
		rlImGuiEnd();
		EndDrawing();
	}
	rlImGuiShutdown();
	CloseWindow();

	return 0;
}