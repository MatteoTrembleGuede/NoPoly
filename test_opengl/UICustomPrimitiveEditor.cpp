#include "UICustomPrimitiveEditor.h"
#include <sstream>
#include "UIResourceBrowser.h"
#include "ShaderGenerator.h"
#include "UIAttribDesc.h"

std::string UIShaderFunctionEditor::noneSelected;

UIShaderFunctionEditor::UIShaderFunctionEditor(std::string _name) : UIWindow(_name)
{
	auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	editor.SetLanguageDefinition(lang);
	noneSelected = "None";

	for (int i = 0; i < AttribType::AT_Count; ++i)
	{
		paramTypes[i] = ToString((AttribType)i).c_str();
	}

	for (int i = 0; i < FunctionType::FT_Count; ++i)
	{
		funcTypes[i] = ToString((FunctionType)i).c_str();
	}
}

void UIShaderFunctionEditor::LoadModel(std::string path, std::string fileName, void* data)
{
	ShaderFunction::GetModel(std::string("/") + UIResourceBrowser::GetRelativePath(path, ShaderGenerator::GetProjectPath()) + fileName);
}

void UIShaderFunctionEditor::SaveLoadModel()
{
	if (currentModel)
	{
		if (ImGui::Button("Save"))
		{
			new UIResourceBrowser("Save Function Model", "funk", currentModel, &ShaderFunctionModel::Save, nullptr, true);
		}
		ImGui::SameLine();
	}
	if (ImGui::Button("Load"))
	{
		new UIResourceBrowser("Load Function Model", "funk", this, &UIShaderFunctionEditor::LoadModel);
	}
}

void UIShaderFunctionEditor::ChooseType()
{
	int oldSelected = currentType;
	int selected = currentType;

	ImGui::Combo("Select type", &selected,
		[](void* data, int idx, const char** out_text)
		{
			std::string* typeList = (std::string*)data;
			if (idx >= 0 && idx < FunctionType::FT_Count)
			{
				*out_text = typeList[idx].c_str();
				return true;
			}
			else
			{
				return false;
			}
		}
	, funcTypes, FunctionType::FT_Count);

	if (selected != oldSelected)
	{
		selectedModel = -1;
		currentType = FunctionType(selected);
		currentModel = nullptr;
	}
}

void UIShaderFunctionEditor::PickModel()
{
	ImGui::SameLine();
	int oldSelected = selectedModel;
	int selected = selectedModel + 1;

	ImGui::Combo("Select model", &selected,
		[](void* data, int idx, const char** out_text)
		{
			idx -= 1;

			if (idx < 0)
			{
				*out_text = const_cast<const char*>(UIShaderFunctionEditor::noneSelected.c_str());
				return true;
			}

			std::list<ShaderFunctionModel*>* modelList = (std::list<ShaderFunctionModel*>*)data;
			if (idx >= 0 && idx < modelList->size())
			{
				auto it = modelList->begin();
				for (int i = 0; i < idx; ++i) ++it;
				*out_text = (*it)->name.c_str();
				return true;
			}
			else
			{
				return false;
			}
		}
	, &ShaderFunction::models[currentType], ShaderFunction::models[currentType].size() + 1);

	if (selected - 1 != oldSelected)
	{
		selectedModel = selected - 1;
		auto it = ShaderFunction::models[currentType].begin();
		for (int i = 0; i < selectedModel; ++i) ++it;
		currentModel = (*it);
		editor.SetText(currentModel->functionBody);
	}
}

void UIShaderFunctionEditor::AddModel()
{
	if (ImGui::Button("Add"))
	{
		currentModel = new ShaderFunctionModel(currentType);
		selectedModel = ShaderFunction::models[currentType].size() - 1;
		editor.SetText(currentModel->functionBody);
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete"))
	{
		if (currentModel)
		{
			bool found = false;
			for (ShaderFunctionModel* model : ShaderFunction::models[currentType])
			{
				if (found) model->ID--;
				else if (model == currentModel) found = true;
			}
			ShaderFunction::models[currentType].remove(currentModel);
			delete currentModel;
			currentModel = nullptr;
			selectedModel = -1;
		}
	}
}

void UIShaderFunctionEditor::SignModel()
{
	ImGui::Text("\n");

	const char* tmpName = currentModel->name.c_str();
	char* newName = new char[512];
	strcpy(newName, tmpName);
	ImGui::InputText("Function Name (no space)\n", newName, 512);
	currentModel->name = newName;
	delete[] newName;

	if (ImGui::Button("Add Parameter"))
	{
		currentModel->parameters.push_back(AttribModel());
		currentModel->onModelChanged();
	}

	ImGui::Text("parameters :");

	switch (currentModel->funcType)
	{
	case FunctionType::SpaceRemap:
		ImGui::Text("mat4 m (local matrix, is added automaticly)");
		ImGui::Text("vec3 point (local space position, is added automaticly)");
		break;
	case FunctionType::Primitive:
		ImGui::Text("out float gradientValue (used by palettes to pick a color)");
		ImGui::Text("vec3 point (local space position, is added automaticly)");
		break;
	}

	if (currentModel->parameters.size() > 0)
	{
		int maxCol = ((int)ImGui::GetWindowSize().x) / 250 + 1;
		if (ImGui::BeginTable("param list : ", ((int)currentModel->parameters.size() < maxCol ? (int)currentModel->parameters.size() : maxCol)))
		{
			int i = 0;
			for (auto it = currentModel->parameters.begin(); it != currentModel->parameters.end(); ++it)
			{
				AttribModel* param = &(*it);
				ImGui::TableNextColumn();
				ImGui::Text("\n");

				int id = param->type;
				if (ImGui::BeginTable("param desc : ", 2))
				{
					ImGui::TableNextColumn();
					ImGui::Combo((std::string("type ") + std::to_string(i)).c_str(), &id,
						[](void* data, int idx, const char** out_text)
						{
							std::string* types = (std::string*)data;
							if (idx >= 0 && idx < AttribType::AT_Count)
							{
								*out_text = types[idx].c_str();
								return true;
							}
							else
							{
								return false;
							}
						}
					, paramTypes, AttribType::AT_Count);

					if (param->type != (AttribType)id)
					{
						param->type = (AttribType)id;
						currentModel->onModelChanged();
						param->oldType = param->type;
					}

					ImGui::TableNextColumn();
					if (ImGui::Button((std::string("options ") + std::to_string(i)).c_str()))
					{
						new UIAttribDesc(param->name, *param);
					}
					ImGui::EndTable();
				}

				const char* tmpName = param->name.c_str();
				char* newName = new char[512];
				strcpy(newName, tmpName);
				ImGui::InputText(("name " + std::to_string(i)).c_str(), newName, 512);
				param->name = newName;
				delete[] newName;

				++i;
			}
			ImGui::EndTable();
		}
	}
}

void UIShaderFunctionEditor::EditFunctionBody()
{
	ImGui::Text("{");
	editor.Render("function body");
	currentModel->functionBody = editor.GetText();
	ImGui::Text("}");
}

void UIShaderFunctionEditor::WindowBody()
{
	UIWindow::WindowBody();

	SaveLoadModel();
	ChooseType();
	AddModel();
	PickModel();

	if (currentModel)
	{
		SignModel();
		EditFunctionBody();
	}
}
