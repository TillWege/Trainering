#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include <iostream>
#include <filesystem>
#include "rlImGuiColors.h"

#define TRAIN_MODELS "models/trains"
#define FLOOR_MODEL "models/terrain/open.obj"

#define screenWidth 1280
#define screenHeight 720

void setup()
{
	SetWindowState(FLAG_VSYNC_HINT);
	InitWindow(screenWidth, screenHeight, "Trainering");
	rlImGuiSetup(true);
}

enum Debug_Menu
{
	DM_MODEL_PREVIEW,
	DM_TRAIN_ASSEMBLY,
};


int main()
{
	setup();

	Camera camera = { 0 };
	camera.position = Vector3 { 10.0f, 10.0f, 10.0f };
	camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
	camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	std::vector<Model> models;
	std::vector<std::string> modelNames;

	for (const auto& entry : std::filesystem::directory_iterator(TRAIN_MODELS)) {
		std::string path = entry.path().string();
		std::string fileName = entry.path().filename().string();
		if (IsFileExtension(fileName.c_str(), ".obj")) {
			Model model = LoadModel(path.c_str());
			models.push_back(model);
			modelNames.push_back(fileName);
		}
	}

	Model floor = LoadModel(FLOOR_MODEL);

	int modelIndex = 0;

	Vector3 modelPosition = { 0.0f, 0.0f, 0.0f };

	auto& io = ImGui::GetIO();


	while (!WindowShouldClose())
	{
		static bool cbOpen = false;
		static int menu = 0;

		struct part
		{
			int modelIndex;
			Vector3 offset;
		};

		static std::vector<part> parts;

		static bool camera_paused = false;

		if (!camera_paused)
			UpdateCamera(&camera, CAMERA_ORBITAL);

		BeginDrawing();

		rlImGuiBegin();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		ImGui::Begin("Camera Controls");
		ImGui::Checkbox("Pause Camera", &camera_paused);
		ImGui::End();

		if(menu == DM_MODEL_PREVIEW)
		{
			DrawModel(models.at(modelIndex), modelPosition, 1.0f, WHITE);
		}
		else if (menu == DM_TRAIN_ASSEMBLY)
		{
			for (auto & part : parts)
			{
				DrawModel(models.at(part.modelIndex), part.offset, 1.0f, WHITE);
			}
		}

		DrawGrid(20, 1.0f);
		static float scale = 1.0;
		static int dim = 1;

		static ImColor color = ImColor(255, 255, 255, 255);

		auto ray = GetMouseRay(GetMousePosition(), camera);
		for (int x = -dim+1; x < dim; x++)
		{
			for (int y = -dim +1; y < dim; y++)
			{
				Vector3 pos = { (float)x, 0.0f, (float)y };
				Vector3 min = { pos.x - 0.5f, pos.y, pos.z - 0.5f };
				Vector3 max = { pos.x + 0.5f, pos.y + 1.0f, pos.z + 0.5f };

				BoundingBox box = { min, max };

				auto rc = GetRayCollisionBox(ray, box);

				if(rc.hit)
					DrawModel(floor, { (float)x, 0.0f, (float)y }, scale, BLUE);
				else
					DrawModel(floor, { (float)x, 0.0f, (float)y }, scale, rlImGuiColors::Convert(color));

			}
		}

		static int debugCubeX = 0;
		static int debugCubeY = 0;

		DrawCube({ (float)debugCubeX, 0.5f, (float)debugCubeY }, 1.0f, 1.0f, 1.0f, RED);

		EndMode3D();

		ImGui::Begin("Floor Controls");
		ImGui::DragFloat("Scale", &scale, 0.1f);
		ImGui::ColorEdit4("Color", (float*)&color);
		ImGui::DragInt("Dim", &dim, 1, 1, 10);

		ImGui::Separator();
		ImGui::Text("Debug Cube");
		ImGui::DragInt("X", &debugCubeX, 1, -10, 10);
		ImGui::DragInt("Y", &debugCubeY, 1, -10, 10);
		ImGui::End();

		ImGui::Begin("Debug Menu");

		if (ImGui::RadioButton("Model Preview", menu == DM_MODEL_PREVIEW)) { menu = DM_MODEL_PREVIEW; } ImGui::SameLine();
		if (ImGui::RadioButton("Train Assembly", menu == DM_TRAIN_ASSEMBLY)) { menu = DM_TRAIN_ASSEMBLY; }


		if (menu == DM_MODEL_PREVIEW)
		{
			ImGui::Text("Model Preview");
			ImGui::Text("Pick Model");

			const char* combo_preview_value = modelNames.at(modelIndex).c_str();
			if (ImGui::BeginCombo("Select Model", combo_preview_value))
			{
				cbOpen = true;
				for (int n = 0; n < models.size(); n++)
				{
					const bool is_selected = (modelIndex == n);
					if (ImGui::Selectable(modelNames.at(n).c_str(), is_selected))
						modelIndex = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			else
			{
				cbOpen = false;
			}
		}
		else if (menu == DM_TRAIN_ASSEMBLY)
		{
			ImGui::Text("Train Assembly");


			if (ImGui::Button("Add Part"))
			{
				part p;
				p.modelIndex = 0;
				p.offset = { 0.0f, 0.0f, 0.0f };
				parts.push_back(p);
			}

			for (int i = 0; i < parts.size(); i++)
			{
				ImGui::Text("Part %d", i);
				const char* combo_part_value = modelNames.at(parts.at(i).modelIndex).c_str();
				std::string cblbl = "Select Model###" + std::to_string(i);
				if (ImGui::BeginCombo(cblbl.c_str(), combo_part_value))
				{
					for (int n = 0; n < models.size(); n++)
					{
						const bool is_selected = (parts.at(i).modelIndex == n);
						if (ImGui::Selectable(modelNames.at(n).c_str(), is_selected))
							parts.at(i).modelIndex = n;

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				std::string df3lbl = "Offset###" + std::to_string(i);
				ImGui::DragFloat3(df3lbl.c_str(), &parts.at(i).offset.x, 0.2f);
			}
		}

		ImGui::End();

		rlImGuiEnd();
		EndDrawing();
	}
	rlImGuiShutdown();
	CloseWindow();

	return 0;
}