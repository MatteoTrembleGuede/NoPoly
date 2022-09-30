#include "UIGlobalParam.h"
#include "ViewportManager.h"
#include "Camera.h"

bool UIGlobalParam::applyPRMNC;
float UIGlobalParam::AORadius;
float UIGlobalParam::AOStrength;
float UIGlobalParam::normalRadius;
float UIGlobalParam::shadowSharpness;

UIGlobalParam::UIGlobalParam(std::string _name, Shader* _shader) : UIWindow(_name), shader(_shader)
{
	AORadius = 1.0f;
	normalRadius = 1.0f;
	AOStrength = 1.5f;
	shadowSharpness = 10.0f;
	applyPRMNC = true;
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

	if (ImGui::Checkbox("Apply Post Raymarching Negative Correction", &applyPRMNC))
	{
		SendGlobalConfig();
	}

	if (ImGui::DragFloat("Resolution Scale", &ViewportManager::screenResolutionScale, 0.01f, 0.01, 1))
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
	shader->setBool("applyPRMNC", applyPRMNC);
}