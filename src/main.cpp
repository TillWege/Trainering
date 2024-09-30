#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include <iostream>
#include <filesystem>
#include "rlImGuiColors.h"
#include "raymath.h"

#define TRAIN_MODELS "models/trains"
#define FLOOR_PLACEHOLDER "models/terrain/open.obj"

#define MARS_FLOORS "models/terrain/mars"
#define FORREST_FLOORS "models/terrain/forrest"
#define CITY_FLOORS "models/terrain/city"

#define screenWidth 1280
#define screenHeight 720

void setup()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(screenWidth, screenHeight, "Trainering");
	rlImGuiSetup(true);
}



struct Tile
{
	Vector2 position;

	int floorTileIndex;
	int contentIndex;
};


std::vector<Model> mars_floors;
std::vector<Model> forrest_floors;
std::vector<Model> city_floors;
Model placeholder_floor;

void loadFloors()
{
	placeholder_floor = LoadModel(FLOOR_PLACEHOLDER);

	for (const auto& entry : std::filesystem::directory_iterator(MARS_FLOORS)) {
		std::string path = entry.path().string();
		std::string fileName = entry.path().filename().string();
		if (IsFileExtension(fileName.c_str(), ".obj")) {
			Model model = LoadModel(path.c_str());
			mars_floors.push_back(model);
		}
	}

	for (const auto& entry : std::filesystem::directory_iterator(FORREST_FLOORS)) {
		std::string path = entry.path().string();
		std::string fileName = entry.path().filename().string();
		if (IsFileExtension(fileName.c_str(), ".obj")) {
			Model model = LoadModel(path.c_str());
			forrest_floors.push_back(model);
		}
	}

	for (const auto& entry : std::filesystem::directory_iterator(CITY_FLOORS)) {
		std::string path = entry.path().string();
		std::string fileName = entry.path().filename().string();
		if (IsFileExtension(fileName.c_str(), ".obj")) {
			Model model = LoadModel(path.c_str());
			city_floors.push_back(model);
		}
	}
}

struct part
{
	int modelIndex;
	Vector3 offset;
};

std::vector<part> parts;

enum GAME_STATE
{
	MODEL_PREVIEW,
	TRAIN_ASSEMBLY,
};

enum BIOME
{
	PLACEHOLDER,
	MARS,
	FORREST,
	CITY
};

struct gamestate
{
	GAME_STATE mode = MODEL_PREVIEW;
	bool cameraPaused = false;

	int modelIndex = 0;
	Vector3 modelPosition = { 0.0f, 0.0f, 0.0f };

	int floor_dim = 10;

	BIOME biome = PLACEHOLDER;
};

gamestate state;


int main()
{
	setup();

	Camera gameCamera = { 0 };
	gameCamera.position = Vector3 { 10.0f, 20.0f, 10.0f };
	gameCamera.target = Vector3{ 0.0f, 0.0f, 0.0f };
	gameCamera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	gameCamera.fovy = 45.0f;
	gameCamera.projection = CAMERA_PERSPECTIVE;

	Camera previewCamera = { 0 };
	previewCamera.position = Vector3{ -5.0f, 5.0f, 0.0f };
	previewCamera.target = Vector3{ 0.0f, 0.0f, 0.0f };
	previewCamera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	previewCamera.fovy = 45.0f;
	previewCamera.projection = CAMERA_PERSPECTIVE;

	std::vector<Model> models;
	std::vector<std::string> modelNames;

	loadFloors();

	for (const auto& entry : std::filesystem::directory_iterator(TRAIN_MODELS)) {
		std::string path = entry.path().string();
		std::string fileName = entry.path().filename().string();
		if (IsFileExtension(fileName.c_str(), ".obj")) {
			Model model = LoadModel(path.c_str());
			models.push_back(model);
			modelNames.push_back(fileName);
		}
	}

	RenderTexture texturePreview = LoadRenderTexture(300, 300);

	bool camera_paused = false;

	while (!WindowShouldClose())
	{
		if (!camera_paused)
			UpdateCamera(&gameCamera, CAMERA_ORBITAL);

		static float scale = 1.0;
		static int floor_idx = 0;

		static ImColor color = ImColor(255, 255, 255, 255);

		BeginTextureMode(texturePreview);
		ClearBackground({ 0, 0, 0, 0 });

		BeginMode3D(previewCamera);

		static float rot = 0;
		rot += 1.0f;

		DrawModelEx(models.at(state.modelIndex),
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			rot,
			{ 1.0f, 1.0f, 1.0f },
			WHITE
		);

		EndMode3D();
		EndTextureMode();

		BeginDrawing();

		ClearBackground(RAYWHITE);

		ClearBackground(RAYWHITE);
		BeginMode3D(gameCamera);

		if(state.mode == MODEL_PREVIEW)
		{
			DrawModel(models.at(state.modelIndex), state.modelPosition, 1.0f, WHITE);
		}
		else if (state.mode == TRAIN_ASSEMBLY)
		{
			for (auto & part : parts)
			{
				DrawModel(models.at(part.modelIndex), part.offset, 1.0f, WHITE);
			}
		}

		DrawGrid(20, 1.0f);

		auto ray = GetMouseRay(GetMousePosition(), gameCamera);

		auto pY = ray.position.y;
		auto dY = ray.direction.y;

		float t = -pY / dY;

		Vector3 hit = Vector3Add(ray.position, Vector3Scale(ray.direction, t));

		int idx = (int)std::round(hit.x);
		int idy = (int)std::round(hit.z);

		for (int x = -state.floor_dim+1; x < state.floor_dim; x++)
		{
			for (int y = -state.floor_dim +1; y < state.floor_dim; y++)
			{
				Model* floor;
				switch(state.biome)
				{
				case PLACEHOLDER:
					floor = &placeholder_floor;
					break;
				case MARS:
					floor = &mars_floors.at(floor_idx);
					break;
				case FORREST:
					floor = &forrest_floors.at(floor_idx);
					break;
				case CITY:
					floor = &city_floors.at(floor_idx);
					break;
				}


				if(x == idx && y == idy)
					DrawModel(*floor, { (float)x, 0.0f, (float)y }, scale, BLUE);
				else
					DrawModel(*floor, { (float)x, 0.0f, (float)y }, scale, rlImGuiColors::Convert(color));
			}
		}

		static int debugCubeX = 0;
		static int debugCubeY = 0;

		DrawCube({ (float)debugCubeX, 0.5f, (float)debugCubeY }, 1.0f, 1.0f, 1.0f, RED);

		EndMode3D();

		DrawText("TESTING", 10, 10, 20, RED);

		rlImGuiBegin();

		ImGui::Begin("Camera Controls");
		ImGui::Checkbox("Pause Camera", &camera_paused);
		ImGui::End();

		ImGui::Begin("Floor Controls");
		ImGui::DragFloat("Scale", &scale, 0.1f);
		ImGui::ColorEdit4("Color", (float*)&color);
		ImGui::DragInt("Dim", &state.floor_dim, 1, 1, 10);

		ImGui::Separator();

		ImGui::Text("Debug Cube");
		ImGui::DragInt("X", &debugCubeX, 1, -10, 10);
		ImGui::DragInt("Y", &debugCubeY, 1, -10, 10);

		ImGui::Separator();
		ImGui::Text("Mouse Hit");
		ImGui::Text("X: %d", idx);
		ImGui::Text("Y: %d", idy);

		ImGui::End();

		ImGui::Begin("Debug Menu");

		if (ImGui::RadioButton("Model Preview", state.mode == MODEL_PREVIEW)) { state.mode = MODEL_PREVIEW; } ImGui::SameLine();
		if (ImGui::RadioButton("Train Assembly", state.mode == TRAIN_ASSEMBLY)) { state.mode = TRAIN_ASSEMBLY; }


		if (state.mode == MODEL_PREVIEW)
		{
			ImGui::Text("Model Preview");
			ImGui::Text("Pick Model");

			const char* combo_preview_value = modelNames.at(state.modelIndex).c_str();
			if (ImGui::BeginCombo("Select Model", combo_preview_value))
			{
				for (int n = 0; n < models.size(); n++)
				{
					const bool is_selected = (state.modelIndex == n);
					if (ImGui::Selectable(modelNames.at(n).c_str(), is_selected))
						state.modelIndex = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			Rectangle viewRect = { 0, 0, 300, -300 };
			rlImGuiImageRect(&texturePreview.texture, (int)300, (int)300, viewRect);
		}
		else if (state.mode == TRAIN_ASSEMBLY)
		{
			ImGui::Text("Train Assembly");


			if (ImGui::Button("Add Part"))
			{
				part p = { 0 };
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

		ImGui::Begin("Floor Control");

		ImGui::Text("Biome");

		static int max_idx = 10;

		if (ImGui::RadioButton("Placeholder", state.biome == PLACEHOLDER)) {
			state.biome = PLACEHOLDER;
		} ImGui::SameLine();

		if (ImGui::RadioButton("Mars", state.biome == MARS)) {
			state.biome = MARS;
			max_idx = (int)mars_floors.size();
		} ImGui::SameLine();

		if (ImGui::RadioButton("Forrest", state.biome == FORREST)) {
			state.biome = FORREST;
			max_idx = (int)forrest_floors.size();
		} ImGui::SameLine();

		if (ImGui::RadioButton("City", state.biome == CITY)) {
			state.biome = CITY;
			max_idx = (int)city_floors.size();
		}

		ImGui::DragInt("Floor Index", &floor_idx, 1, 0, max_idx - 1);


		ImGui::End();

		rlImGuiEnd();

		EndDrawing();
	}
	rlImGuiShutdown();
	CloseWindow();

	return 0;
}