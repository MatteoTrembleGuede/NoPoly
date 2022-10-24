#include "UIHost.h"
#include "ViewportManager.h"
#include <iostream>

UIHost::UIHost(std::string _name) : UIWindow(_name)
{
	flags = ImGuiWindowFlags_NoTitleBar;
	static int hostId = 0;
	name += " ";
	name += std::to_string(hostId);
	++hostId;
}

UIHost::~UIHost()
{
	if (((UIWindow*)this)->snapSection && children.size() > 0) ((UIWindow*)this)->snapSection = nullptr;
}

void UIHost::AddChild(UIWindow* window)
{
	mustSkipDisplay = true;
	windows.remove(window);
	//ViewportManager::RemoveSection(window->snapSection);
	window->snapSection = nullptr;
	children.push_back(window);
	window->host = this;

	UIHost* childHost = dynamic_cast<UIHost*>(window);

	if (childHost)
	{
		for (UIWindow*& child : childHost->children)
		{
			children.push_back(child);
		}
		children.remove(childHost);
		if (ViewportManager::draggedWindow == childHost) ViewportManager::DragWindow(this);
		delete childHost;
	}
	mustSkipDisplay = true;
}

void UIHost::RemoveChild(UIWindow* window)
{
	UIWindow::windows.push_back(window);
	window->host = nullptr;
	children.remove(window);
	mustSkipDisplay = true;
}

void UIHost::WindowBody()
{
	UIWindow::WindowBody();

	if (ImGui::BeginTabBar("children", ImGuiTabBarFlags_Reorderable))
	{
		ImVec2 size, pos, mousePos;
		pos = ImGui::GetWindowPos();
		size = ImGui::GetWindowSize();
		mousePos = ImGui::GetMousePos();
		bool out = mousePos.x < pos.x || mousePos.x > pos.x + size.x || mousePos.y < pos.y || mousePos.y > pos.y + size.y;

		int i = 1;
		for (UIWindow*& window : children)
		{
			if (ImGui::BeginTabItem(window->name.c_str(), nullptr))
			{
				if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0, 0.1))
				{
					if (out)
					{
						ViewportManager::DragWindow(window);
					}
				}
				ImGui::BeginChild(i);
				window->WindowBody();
				ImGui::EndChild();
				ImGui::EndTabItem();
			}

			if (mustSkipDisplay) break;

			++i;
		}
		ImGui::EndTabBar();
	}

	if (children.size() == 1)
	{
		for (UIWindow*& child : children)
		{
			child->snapSection = ((UIWindow*)this)->snapSection;
			child->host = nullptr;
			UIWindow::windows.push_back(child);
		}
		children.clear();
		((UIWindow*)this)->snapSection = nullptr;
	}
	if (children.size() == 0) Destroy();
}
