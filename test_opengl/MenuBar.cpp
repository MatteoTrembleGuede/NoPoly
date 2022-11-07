#include "MenuBar.h"
#include "UIResourceBrowser.h"
#include "UIDisplayTextWindow.h"
#include "ShaderGenerator.h"
#include "ViewportManager.h"
#include "Guizmo.h"
#include "UIShaderEditorWindow.h"
#include "UIKeyBinding.h"

void MenuBar::Display()
{
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		std::string path = std::string("Save/");
		if (ImGui::MenuItem("New"))
		{
			editorWindow->shader->Clear();
			editorWindow->SetPartTarget((ShaderPart*)editorWindow->shader->root);
			editorWindow->compilationError = !editorWindow->shader->Recompile(editorWindow->errorMsg, editorWindow->errorCode);
			editorWindow->editor.SetText(editorWindow->errorCode);
		}
		else if (ImGui::MenuItem("Save"))
		{
			new UIResourceBrowser("Save", ".scene", editorWindow, &UIShaderEditorWindow::Save, nullptr, true);
		}
		if (!editorWindow->isLeaf && (ShaderNode*)editorWindow->currentPart != editorWindow->shader->root && ImGui::MenuItem("Save from this node"))
		{
			new UIResourceBrowser("Save from this node : " + editorWindow->currentPart->name, ".scene", editorWindow, &UIShaderEditorWindow::Save, (ShaderNode*)editorWindow->currentPart, true);
		}
		if (ImGui::MenuItem("Load"))
		{
			new UIResourceBrowser("Load", ".scene", editorWindow, &UIShaderEditorWindow::Load, nullptr);
		}
		if (!editorWindow->isLeaf && (ShaderNode*)editorWindow->currentPart != editorWindow->shader->root && ImGui::MenuItem("Load from this node"))
		{
			new UIResourceBrowser("Load from this node : " + editorWindow->currentPart->name, ".scene", editorWindow, &UIShaderEditorWindow::Load, (ShaderNode**)&editorWindow->currentPart);
		}
		//if (ImGui::MenuItem("Quit", "Esc"))
			// QUIT   
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("About"))
	{
		if (ImGui::MenuItem("Useful keys"))
		{
			new UIKeyBinding("shortcuts");
			/*new UIDisplayTextWindow("Useful keys",
				"	Global controls\n"
				"H : hide ui\n"
				"G : hide gizmo\n"
				"R : start recording (must be in loop or mirrored loop playing mode)\n"
				"Space : pause/resume\n"
				"Numpad Enter : compile\n"
				"Numpad 1/2/3 : translation/rotation/scale gizmo\n"
				"Numpad 0 : switch world/local gizmo\n\n"
				"	Camera controls\n"
				"Right click : camera control mode\n"
				"Left alt : switch qwerty/azerty\n"
				"Z/W : forward	S : backward\n"
				"Q/A : left		D : right\n"
				"A/Q : down		E : up\n"
				"Mouse wheel : change camera move speed\n"
				"Backspace : reset camera position and speed\n"
			);*/
		}

		if (ImGui::MenuItem("Credits"))
		{
			new UIDisplayTextWindow("Credits",
				"NoPoly : Matteo TREMBLE-GUEDE\n\n\n"
				"	UI Libraries\n\n"
				"Dear ImGui : Omar Cornut\n"
				"	https://github.com/ocornut/imgui \n\n"
				"ImGuizmo : Cedric Guillemet\n"
				"	https://github.com/CedricGuillemet/ImGuizmo \n\n"
				"Text editor : Balazs Jako\n"
				"	https://github.com/BalazsJako/ImGuiColorTextEdit \n\n\n"
				"	Shader Algorithms\n\n"
				"Signed distance functions : Inigo Quilez\n"
				"	https://iquilezles.org \n"
				"	(very useful blog to learn about raymarching)\n\n"
				"Simplex noise : Nikita Miropolskiy\n"
				"	http://www.shadertoy.com/view/XsX3zB \n\n"
			);
		}
		ImGui::EndMenu();
	}
	ImVec2 size;
	ViewportManager::GetScreenSize(size.x, size.y);
	ImGui::SetCursorPosX(size.x - 200.0f);
	ImGui::Text((std::string("Guizmo : ") + std::string(Guizmo::IsWorld() ? "World " : "local ")).c_str());
	ImGui::EndMainMenuBar();
}
