#include "Guizmo.h"
#include "imgui/imgui.h"
#include "ImGuizmo/ImGuizmo.h"
#include "Camera.h"
#include "ShaderPart.h"
#include "Transform.h"
#include "ViewportManager.h"
#include "Input/Input.h"

Transformable* Guizmo::target;
bool Guizmo::isVisible;
int Guizmo::manipulationOP;
int Guizmo::manipulationMode;

void SetTranslateMode()
{
	Guizmo::SetOperation((int)ImGuizmo::TRANSLATE);
}

void SetRotateMode()
{
	Guizmo::SetOperation((int)ImGuizmo::ROTATE);
}

void SetScaleMode()
{
	Guizmo::SetOperation((int)ImGuizmo::SCALE);
}

void ToggleWorldLocal()
{
	Guizmo::SetWorld(!Guizmo::IsWorld());
}

void ToggleVisibility()
{
	Guizmo::SetVisible(!Guizmo::IsVisible());
}

void Guizmo::Init()
{
	target = nullptr;
	isVisible = true;
	manipulationOP = ImGuizmo::TRANSLATE;
	manipulationMode = ImGuizmo::WORLD;

	Input::GetGlobalInput(0).AddAction("SetTranslate", Input::Key(Input::KeyVal::KP_1));
	Input::GetGlobalInput(0).BindAction("SetTranslate", Input::Mode::Press, &SetTranslateMode);

	Input::GetGlobalInput(0).AddAction("SetRotate", Input::Key(Input::KeyVal::KP_2));
	Input::GetGlobalInput(0).BindAction("SetRotate", Input::Mode::Press, &SetRotateMode);

	Input::GetGlobalInput(0).AddAction("SetScale", Input::Key(Input::KeyVal::KP_3));
	Input::GetGlobalInput(0).BindAction("SetScale", Input::Mode::Press, &SetScaleMode);

	Input::GetGlobalInput(0).AddAction("ToggleWorldLocal", Input::Key(Input::KeyVal::KP_0));
	Input::GetGlobalInput(0).BindAction("ToggleWorldLocal", Input::Mode::Press, &ToggleWorldLocal);

	Input::GetGlobalInput(0).AddAction("ToggleVisibility", Input::Key(Input::KeyVal::G));
	Input::GetGlobalInput(0).BindAction("ToggleVisibility", Input::Mode::Press, &ToggleVisibility);
}

void Guizmo::Update(Camera* camera)
{
	if (isVisible && target)
	{
		glm::mat4 identity = glm::mat4(1.0f);
		ImVec2 sceneViewPos, sceneViewSize, mousePos;
		ViewportManager::GetSceneViewPosAndSize(sceneViewSize, sceneViewPos);
		mousePos = ImGui::GetMousePos();

		if ((mousePos.x > sceneViewPos.x && mousePos.x < (sceneViewPos.x + sceneViewSize.x)
			&& mousePos.y > sceneViewPos.y && mousePos.y < (sceneViewPos.y + sceneViewSize.y))
			&& target->IsDynamic())
		{
			ImGuizmo::Enable(true);
		}
		else
		{
			ImGuizmo::Enable(false);
		}

		glm::mat4 finalMatrix;
		glm::mat4 view = camera->GetView();
		glm::mat4 projection = camera->GetProjection();


		ImGuizmo::SetRect(sceneViewPos.x, sceneViewPos.y, sceneViewSize.x, sceneViewSize.y);
		//ImGuizmo::DrawGrid((float*)&view, (float*)&projection, (float*)&identity, 20);
		if (!ImGui::IsAnyItemActive())
		{
			glm::vec3 position = target->GetPosition();
			glm::vec3 rotation = target->GetRotation();
			glm::vec3 scale = target->GetScale();
			ImGuizmo::RecomposeMatrixFromComponents((float*)&position, (float*)&rotation, (float*)&scale, (float*)&finalMatrix);
			view = camera->GetInvView(target->GetLocalBase());
			ImGuizmo::Manipulate((float*)&view, (float*)&projection, (ImGuizmo::OPERATION)manipulationOP, (ImGuizmo::MODE)manipulationMode, (float*)&finalMatrix);
			ImGuizmo::DecomposeMatrixToComponents((float*)&finalMatrix, (float*)&position, (float*)&rotation, (float*)&scale);

			target->SetPosition(position);
			target->SetRotation(rotation);
			target->SetScale(scale);
		}
	}
}

void Guizmo::SetTarget(Transformable* newTarget)
{
	target = newTarget;
}

void Guizmo::SetVisible(bool visible)
{
	isVisible = visible;
}

void Guizmo::SetOperation(int operation)
{
	manipulationOP = operation;
}

void Guizmo::SetWorld(bool world)
{
	manipulationMode = (int)world;
}

bool Guizmo::IsWorld()
{
	return manipulationMode == ImGuizmo::WORLD;
}
