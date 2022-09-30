#include "UIDisplayTextWindow.h"

UIDisplayTextWindow::UIDisplayTextWindow(std::string _name, std::string _text) : UIWindow(_name)
{
	editor.SetReadOnly(true);
	editor.SetColorizerEnable(false);
	editor.SetShowWhitespaces(false);
	editor.SetText(_text);
}

UIDisplayTextWindow::~UIDisplayTextWindow()
{

}

void UIDisplayTextWindow::WindowBody()
{
	ImGui::SetWindowSize(ImVec2(460, 410));
	UIWindow::WindowBody();

	editor.Render("", ImVec2(450, 350));

	if (ImGui::Button("Close"))
	{
		Destroy();
	}
}
