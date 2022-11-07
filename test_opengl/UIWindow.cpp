#include "UIWindow.h"
#include "ViewportManager.h"


std::list<UIWindow*> UIWindow::windows;
std::list<UIWindow*> UIWindow::notHidableWindows;
bool UIWindow::mustSkipDisplay;

UIWindow::UIWindow() : notHidable(false)
{
	flags = 0;
	name = "Untitled";
	windows.push_front(this);
	snapSection = nullptr;
}

UIWindow::UIWindow(std::string _name, bool _notHidable) : notHidable(_notHidable)
{
	name = _name;

	if (notHidable)
		notHidableWindows.push_front(this);
	else
		windows.push_front(this);

	snapSection = nullptr;
}

UIWindow::~UIWindow()
{
	if (snapSection) ViewportManager::RemoveSection(snapSection);

	if (notHidable)
		notHidableWindows.remove(this);
	else
		windows.remove(this);

	mustSkipDisplay = true;
}

void UIWindow::Display()
{
	ImGui::Begin(name.c_str(), nullptr, flags | (snapSection ? ImGuiWindowFlags_NoMove : 0));
	WindowBody();
	ImGui::End();

	if (isDestroyed) delete this;
}

void UIWindow::DisplayUI()
{
	for (auto it = notHidableWindows.begin(); it != notHidableWindows.end(); ++it)
	{
		(*it)->Display();

		if (MustSkip()) return;
	}

	if (ViewportManager::IsSceneViewMaximized()) return;

	for (auto it = windows.begin(); it != windows.end(); ++it)
	{
		(*it)->Display();

		if (MustSkip()) return;
	}
}

UIWindow* UIWindow::GetWindow(std::string name)
{
	for (auto it = windows.begin(); it != windows.end(); ++it)
	{
		if ((*it)->name == name)
		{
			return (*it);
		}
	}

	return nullptr;
}

bool UIWindow::MustSkip()
{
	bool result = mustSkipDisplay;
	mustSkipDisplay = false;
	return result;
}

void UIWindow::WindowBody()
{
	if (ViewportManager::draggedWindow == this) return;
	if (snapSection)
	{
		ImVec2 pos, size;
		snapSection->GetSizeAndPosition(size, pos);

		if (!isResized)
		{
			ImGui::SetWindowPos(pos);
			ImGui::SetWindowSize(size);
		}

		ImVec2 mousePos = ImGui::GetMousePos();

		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsWindowHovered() &&
				ImGui::IsMouseDragging(0, 0.1f) && 
				!ViewportManager::draggedWindow)
			{
				ViewportManager::DragWindow(this);
			}
			if (snapSection)
			{
				ImVec2 previousSize = ImGui::GetWindowSize();
				ImVec2 previousPos = ImGui::GetWindowPos();
				if (!ImGui::IsWindowHovered() && ImGui::IsMouseDragging(0, 0.1f))
				{
					if (!canResize) return;
					float value;
					ImVec2 reSize2 = ImGui::GetWindowSize();
					Vector3 reSize = Vector3(reSize2.x, reSize2.y, 0);
					Vector3 sideDragRefSize = Vector3(size.x, size.y, 0);
					Vector3 sizeDif = reSize - sideDragRefSize;

					if (!isResized)
					{
						if (ImGui::IsMouseHoveringRect(ImVec2(previousPos.x + 2, previousPos.y + 20), ImVec2(previousPos.x + previousSize.x - 2, previousPos.y + previousSize.y - 5)))
						{
							canResize = false;
							return;
						}
						sideDragged = snapSection->GetDirectionFromSectionCenter(mousePos);
						isResized = true;
					}

					if (sideDragged == ViewportSnapSectionQuadrant::right || sideDragged == ViewportSnapSectionQuadrant::left)
						value = sizeDif.x;
					else
						value = sizeDif.y;

					snapSection->PushSide(sideDragged, value);
				}
				else if (isResized)
				{
					isResized = false;
				}
				else
				{
					canResize = true;
				}
			}
		}
	}
}

void UIWindow::Destroy()
{
	isDestroyed = true;
}
