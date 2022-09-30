#include "UIMaterialInspector.h"
#include "Material.h"
#include "Texture.h"
#include "UIResourceBrowser.h"

UIMaterialInspector::UIMaterialInspector(std::string _name) : UIWindow(_name)
{
	currentMat = nullptr;
	Material::onClear.Bind(this, &UIMaterialInspector::ClearSelection);
}

UIMaterialInspector::~UIMaterialInspector()
{
	Material::onClear.Unbind(this, &UIMaterialInspector::ClearSelection);
}

void UIMaterialInspector::LoadTextureTo(std::string path, std::string fileName, void* data)
{
	if (!data) return;

	Texture** tex = (Texture**)data;

	if (*tex)
	{
		delete (*tex);
	}

	*tex = Texture::LoadFromFile((path + fileName).c_str());

	if (autoMerge)
	{
		currentMat->Merge();
	}
	else
	{
		currentMat->needMerge = true;
		Material::needMergeBeforeSending = true;
	}
}

void UIMaterialInspector::SetTexture(std::string name, Texture** tex)
{
	if (ImGui::Button(("Browse " + name).c_str()))
	{
		new UIResourceBrowser("Load texture : " + name, "jpeg,jpg,png,tga,bmp", this, &UIMaterialInspector::LoadTextureTo, tex);
	}

	ImGui::SameLine();

	if (*tex)
	{
		ImGui::Image((ImTextureID)(*tex)->GetGlID(), ImVec2(100.0f, 100.0f));

		ImGui::SameLine();
		if (ImGui::Button(("discard " + name).c_str()))
		{
			delete *tex;
			*tex = nullptr;
			currentMat->Merge();
		}
	}
	else
	{
		ImGui::Image(0, ImVec2(100.0f, 100.0f));
	}
}

void UIMaterialInspector::DisplayFinalTexture()
{
	if (currentMat)
	{
		ImGui::Checkbox("Auto Merge", &autoMerge);
		if (!autoMerge)
		{
			if (ImGui::Button("Merge"))
			{
				currentMat->Merge();
			}

			ImGui::SameLine();
		}

		if (currentMat->finalTexture)
		{
			ImGui::Image((ImTextureID)currentMat->finalTexture->GetGlID(), ImVec2(200.0f, 200.0f));
		}
		else
		{
			ImGui::Image(0, ImVec2(200.0f, 200.0f));
		}
	}
}

void UIMaterialInspector::InputName()
{
	const char* tmpName = currentMat->name.c_str();
	char* newName = new char[128];
	strcpy(newName, tmpName);
	ImGui::InputText("name", newName, 128);
	currentMat->name = newName;
	delete[] newName;
}

void UIMaterialInspector::SelectMaterial()
{
	if (ImGui::Button("New"))
	{
		Material* newMat = Material::Create();
		if (newMat)
		{
			currentMat = newMat;
			selectedMaterial = Material::materials.size() - 1;
		}
	}

	int oldSelected = selectedMaterial;

	ImGui::Combo("Select material", &selectedMaterial,
		[](void* data, int idx, const char** out_text)
		{
			std::list<Material*>* matList = (std::list<Material*>*)data;
			if (idx >= 0 && idx < matList->size())
			{
				auto it = matList->begin();
				for (int i = 0; i < idx; ++i) ++it;
				*out_text = (*it)->name.c_str();
				return true;
			}
			else
			{
				return false;
			}
		}
	, &Material::materials, Material::materials.size());

	if (selectedMaterial != oldSelected)
	{
		auto it = Material::materials.begin();
		for (int i = 0; i < selectedMaterial; ++i) ++it;
		currentMat = *it;
	}
}

void UIMaterialInspector::SendData()
{
	if (ImGui::Button("send to shader"))
	{
		if (Material::materials.size() > 0)
		{
			Material::SendData(shader);
		}
	}
}

void UIMaterialInspector::WindowBody()
{
	UIWindow::WindowBody();

	SendData();
	ImGui::SameLine();
	SelectMaterial();
	ImGui::Text("\n\n");
	
	if (currentMat)
	{
		InputName();
		SetTexture("diffuse", &currentMat->diffuse);
		SetTexture("normal", &currentMat->normal);
		SetTexture("roughness", &currentMat->roughness);
		SetTexture("ambient occlusion", &currentMat->ambientOcclusion);
		DisplayFinalTexture();
	}
}

void UIMaterialInspector::ClearSelection()
{
	currentMat = nullptr;
	selectedMaterial = -1;
	autoMerge = false;
}
