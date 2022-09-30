#include "UIPopUp.h"

UIPopUp::UIPopUp() : UIWindow()
{

}

UIPopUp::UIPopUp(std::string _name) : UIWindow(_name)
{

}

UIPopUp::~UIPopUp()
{

}

void UIPopUp::Display()
{
	ImGui::OpenPopup(name.c_str());
	ImGui::BeginPopup(name.c_str());
	WindowBody();
	ImGui::EndPopup();
}

void UIPopUp::WindowBody()
{
	ImGui::Text("test pop up");

	if (ImGui::Button("close pop up"))
	{
		delete this;
	}
}
