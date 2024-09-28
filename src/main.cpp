#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include <iostream>
#include <filesystem>

#define MODEL_FOLDER "models/"

int main()
{
	const int screenWidth = 1280;
	const int screenHeight = 720;

	SetWindowState(FLAG_VSYNC_HINT);
	InitWindow(screenWidth, screenHeight, "Trainering");
	rlImGuiSetup(true);

	Camera camera = { 0 };
	camera.position = Vector3 { 10.0f, 10.0f, 10.0f };
	camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
	camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	std::vector<Model> models;
	std::vector<std::string> modelNames;

	for (const auto& entry : std::filesystem::directory_iterator(MODEL_FOLDER)) {
		std::string path = entry.path().string();
		std::string fileName = entry.path().filename().string();
		if (IsFileExtension(fileName.c_str(), ".obj")) {
			Model model = LoadModel(path.c_str());
			models.push_back(model);
			modelNames.push_back(fileName);
		}
	}

	int modelIndex = 0;

	Vector3 modelPosition = { 0.0f, 0.0f, 0.0f };

	auto& io = ImGui::GetIO();

	enum Debug_Menu
	{
		DM_MODEL_PREVIEW,
		DM_TRAIN_ASSEMBLY,
	};

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

		if(!cbOpen && (menu == DM_MODEL_PREVIEW))
			UpdateCamera(&camera, CAMERA_ORBITAL);

		BeginDrawing();

		rlImGuiBegin();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);


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

		EndMode3D();

		ImGui::Begin("Trainering");



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
				ImGui::DragFloat3(df3lbl.c_str(), &parts.at(i).offset.x, 0.1f);
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