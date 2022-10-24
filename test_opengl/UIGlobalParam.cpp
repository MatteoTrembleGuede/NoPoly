#include "UIGlobalParam.h"
#include "ViewportManager.h"
#include "Camera.h"

NegativeCorrectionMode UIGlobalParam::correctionMode;
int UIGlobalParam::NCStepNum;
int UIGlobalParam::DRMNCPrecision;
float UIGlobalParam::AORadius;
float UIGlobalParam::AOStrength;
float UIGlobalParam::normalRadius;
float UIGlobalParam::shadowSharpness;

UIGlobalParam::UIGlobalParam(std::string _name, Shader* _shader) : UIWindow(_name), shader(_shader)
{
	NCStepNum = 1;
	DRMNCPrecision = 1;
	AORadius = 1.0f;
	normalRadius = 1.0f;
	AOStrength = 1.5f;
	shadowSharpness = 10.0f;
	correctionMode = None;
	shader->onCompilation.Bind(this, &UIGlobalParam::SendGlobalConfig);
}

UIGlobalParam::~UIGlobalParam()
{
	shader->onCompilation.Unbind(this, &UIGlobalParam::SendGlobalConfig);
}

void UIGlobalParam::WindowBody()
{
	UIWindow::WindowBody();

	if (ImGui::DragFloat("Normal Radius", &normalRadius, 0.1f, 0, FLT_MAX))
	{
		SendGlobalConfig();
	}

	if (ImGui::DragFloat("Ambient Occlusion Radius", &AORadius, 0.01f, 0, FLT_MAX))
	{
		SendGlobalConfig();
	}

	if (ImGui::DragFloat("Ambient Occlusion Strength", &AOStrength, 0.01f, 0, FLT_MAX))
	{
		SendGlobalConfig();
	}

	if (ImGui::DragFloat("Field Of View", &Camera::fov, 0.01f, 0, FLT_MAX))
	{
		SendGlobalConfig();
	}

	if (ImGui::DragFloat("Shadow Sharpness", &shadowSharpness, 0.01f, 0, FLT_MAX))
	{
		SendGlobalConfig();
	}

	int selected = (int)correctionMode;
	std::string modes[3] = { "None", "Post Raymarching (quite heavy)", "During Raymarching (very heavy)" };
	ImGui::Combo("Negative Correction Mode", &selected,
		[](void* data, int idx, const char** out_text)
		{
			std::string* typeList = (std::string*)data;
			if (idx >= 0 && idx < NegativeCorrectionMode::NCCount)
			{
				*out_text = typeList[idx].c_str();
				return true;
			}
			else
			{
				return false;
			}
		}
	, modes, NegativeCorrectionMode::NCCount);

	if (selected != correctionMode)
	{
		correctionMode = (NegativeCorrectionMode)selected;
		SendGlobalConfig();
	}

	if (correctionMode != NegativeCorrectionMode::None)
	{
		if (ImGui::DragInt("Max correction step number", &NCStepNum, 1, 1, INT_MAX))
		{
			SendGlobalConfig();
		}

		if (correctionMode == NegativeCorrectionMode::During && 
			ImGui::DragInt("Correction precision (during raymarching)", &DRMNCPrecision, 1, 1, 3))
		{
			SendGlobalConfig();
		}
	}

	if (ImGui::DragFloat("Resolution Scale", &ViewportManager::screenResolutionScale, 0.01f, 0.01f, 1))
	{
		ViewportManager::AdjustQuad();
	}
}

void UIGlobalParam::SendGlobalConfig()
{
	shader->setFloat("AORadius", AORadius);
	shader->setFloat("AOStrength", AOStrength);
	shader->setFloat("normalRadius", normalRadius);
	shader->setFloat("shadowSharpness", shadowSharpness);
	shader->setBool("applyDRMNC", correctionMode == During);
	shader->setBool("applyPRMNC", correctionMode >= Post);
	shader->setInt("PRMNCStepNum", NCStepNum);
	shader->setInt("DRMNCPrecision", DRMNCPrecision);
}