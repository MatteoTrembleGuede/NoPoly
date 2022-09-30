#include "UIDebug.h"
#include "Shader.h"

bool UIDebug::showTrespassing;
bool UIDebug::showSDF;
bool UIDebug::showComplexity;
bool UIDebug::showRenderChunks;

void UIDebug::SendDebugConfig()
{
	shader->setBool("showSDF", showSDF); 
	shader->setBool("showComplexity", showComplexity);
	shader->setBool("showTrespassing", showTrespassing);
	shader->setBool("showRenderChunks", showRenderChunks);
}

UIDebug::UIDebug(std::string _name, Shader* _shader) : UIWindow(_name)
{
	shader = _shader;
	shader->onCompilation.Bind(this, &UIDebug::SendDebugConfig);
}

UIDebug::~UIDebug()
{
	shader->onCompilation.Unbind(this, &UIDebug::SendDebugConfig);
}

void UIDebug::WindowBody()
{
	UIWindow::WindowBody();

	if (ImGui::Checkbox("Display SDF", &showSDF))
	{
		SendDebugConfig();
	}

	if (ImGui::Checkbox("Show Trespassing", &showTrespassing))
	{
		SendDebugConfig();
	}

	if (ImGui::Checkbox("Show Complexity", &showComplexity))
	{
		SendDebugConfig();
	}

	if (ImGui::Checkbox("Show Render Chunks", &showRenderChunks))
	{
		SendDebugConfig();
	}
}
