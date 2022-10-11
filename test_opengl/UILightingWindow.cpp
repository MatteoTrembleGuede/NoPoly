#include "UILightingWindow.h"
#include "Lighting.h"
#include "Light.h"
#include "Guizmo.h"

#define STR(a) std::to_string(a)

UILightingWindow::UILightingWindow(std::string _name, Lighting* _lighting) : UIWindow(_name)
{
	lighting = _lighting;
	currentLight = nullptr;
}

UILightingWindow::~UILightingWindow()
{

}

void UILightingWindow::SunDirection()
{
	ImGui::DragFloat3("sun direction (x/y/z)", (float*)&lighting->sunDirection, 0.1f, -1.0f, 1.0f);
}

void UILightingWindow::SunColor()
{
	ImGui::ColorEdit3("sun color", (float*)&lighting->sunColor);
}

void UILightingWindow::SkyColor()
{
	ImGui::ColorEdit3("sky color", (float*)&lighting->skyColor);
}

void UILightingWindow::AmbientColor()
{
	ImGui::ColorEdit3("ambient color", (float*)&lighting->ambientColor);
}

void UILightingWindow::FloorColor()
{
	ImGui::ColorEdit3("floor indirect coloration", (float*)&lighting->floorColor);
}

void UILightingWindow::LightPicker()
{
	ImGui::Text("\n\n");

	int id = 0;
	int width = ImGui::GetWindowSize().x;

	ImGui::Text("Light Picker : \n\n");
	ImGui::SameLine();
	if (ImGui::Button("New Light"))
	{
		Light::Create();
	}

	ImGui::Text("\n");

	for (auto it = Light::lights.begin(); it != Light::lights.end(); ++it)
	{
		Light* light = *it;
		ImGui::BeginChild(id + 1, ImVec2(width, 20));
		if (ImGui::BeginTable("Light Picker : ", 2))
		{

			ImGui::TableNextColumn();

			if (ImGui::Button(light->name.c_str()))
			{
				currentLight = light;
			}

			ImGui::TableNextColumn();

			if (ImGui::Button("delete"))
			{
				if (light == currentLight) currentLight = nullptr;
				Light::Destroy(light);
				light = nullptr;
			}

			ImGui::EndTable();
		}
		++id;
		ImGui::EndChild();
		if (!light) break;
	}
}

void UILightingWindow::InputName()
{
	const char* tmpName = currentLight->name.c_str();
	char* newName = new char[128];
	strcpy(newName, tmpName);
	ImGui::InputText("name", newName, 128);
	currentLight->name = newName;
	delete[] newName;
}

void UILightingWindow::ChangeLightType()
{
	int selectedType = (int)currentLight->type - 1;
	const char* types[3]{ "Directional" , "Point" , "Spot" };

	ImGui::Combo("Light Type", &selectedType, types, 3);

	if (selectedType + 1 != (int)currentLight->type)
	{
		currentLight->type = (LightType)(selectedType + 1);
	}

	ImGui::Text("\n");
}

void UILightingWindow::CommonLightData()
{
	InputName();
	ChangeLightType();
	ImGui::DragFloat3("position (x/y/z)", (float*)&currentLight->position, 0.1f, -FLT_MAX, FLT_MAX);
	ImGui::DragFloat3("direction (x/y/z)", (float*)&currentLight->direction, 0.1f, -1.0f, 1.0f);

	if (currentLight->direction.x != 0.0f || currentLight->direction.y != 0.0f || currentLight->direction.z != 0.0f)
	{
		currentLight->direction.Normalize();
	}

	ImGui::ColorEdit3("color", (float*)&currentLight->color);
	ImGui::DragFloat("intensity", (float*)&currentLight->intensity, 0.1f, -FLT_MAX, FLT_MAX);
}

void UILightingWindow::PointLight()
{
	ImGui::DragFloat("radius", (float*)&currentLight->radius, 0.1f, 0.0001f, FLT_MAX);
	ImGui::DragFloat("attenuation", (float*)&currentLight->attenuation, 0.1f, 0.0001f, FLT_MAX);
}

void UILightingWindow::SpotLight()
{
	PointLight();
	ImGui::DragFloat("aperture", (float*)&currentLight->aperture, 0.1f, 0.0001f, 3.1415f);
	ImGui::DragFloat("focus", (float*)&currentLight->focus, 0.1f, 0.0001f, FLT_MAX);
}

void UILightingWindow::LightInspector()
{
	ImGui::Text("\n\nLight Inspector : \n");

	if (currentLight)
	{
		CommonLightData();

		switch (currentLight->type)
		{
		case 2:
			PointLight();
			break;
		case 3:
			SpotLight();
			break;
		}
	}
}

void UILightingWindow::WindowBody()
{
	UIWindow::WindowBody();

	if (ImGui::IsWindowFocused())
	{
		Guizmo::SetTarget(currentLight);
	}

	ImGui::Text("Global Lighting : \n");
	SunDirection();
	SunColor();
	SkyColor();
	AmbientColor();
	FloorColor();
	LightPicker();
	LightInspector();
}
