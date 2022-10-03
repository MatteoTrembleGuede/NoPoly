#include "UIAttribDesc.h"

void UIAttribDesc::WindowBody()
{
	ImGui::Checkbox("minimum", &attrib.hasMin);
	if (attrib.hasMin)
	{
		ImGui::InputFloat("minimum value", &attrib.minimum);
	}

	ImGui::Checkbox("maximum", &attrib.hasMax);
	if (attrib.hasMax)
	{
		ImGui::InputFloat("maximum value", &attrib.maximum);
	}

	ImGui::Checkbox("default", &attrib.hasDef);
	if (attrib.hasDef)
	{
		ImGui::InputFloat("default value", &attrib.defaultVal);
	}

	ImGui::Checkbox("step", &attrib.hasStep);
	if (attrib.hasStep)
	{
		ImGui::InputFloat("step value", &attrib.step);
	}

	if (ImGui::Button("Close"))
	{
		delete this;
	}
}
