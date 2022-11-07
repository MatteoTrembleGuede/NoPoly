#include "UIKeyBinding.h"

UIKeyBinding::UIKeyBinding(std::string _name) : UIWindow(_name)
{
	backup = Input::GetGlobalInput(0).GetBindSet();
}

void UIKeyBinding::DisplayActions(Input::BindSet& bs)
{
	float width = ImGui::GetWindowSize().x / 2.0f;
	int half = ceilf(bs.GetActions().size() / 2.0f);
	if (ImGui::BeginTable("Actions", 2))
	{
		ImGui::TableNextColumn();
		int i = 0;
		for (Input::BindSet::ActionDescriptor& act : bs.GetActions())
		{
			if (i == half) ImGui::TableNextColumn();

			ImGui::BeginChild(i + 1, ImVec2(width, 25));

			if (ImGui::BeginTable("keys", 2))
			{
				ImGui::TableNextColumn();
				ImGui::Text(act.name.c_str());
				ImGui::TableNextColumn();
				if (ImGui::Button(act.ToString().c_str() + act.name.size() + 1))
				{
					Input::GetGlobalInput(0).SetActionListening(act.name);
				}
				ImGui::EndTable();
			}

			ImGui::EndChild();
			++i;
		}
		ImGui::EndTable();
	}
}

void UIKeyBinding::DisplayAxes(Input::BindSet& bs)
{
	float width = ImGui::GetWindowSize().x / 2.0f;
	int half = ceilf(bs.GetAxes().size() / 2.0f);
	int actionsNum = bs.GetActions().size();
	if (ImGui::BeginTable("Axes", 2))
	{
		ImGui::TableNextColumn();
		int i = 0;
		for (Input::BindSet::AxisDescriptor& ax : bs.GetAxes())
		{
			if (i == half) ImGui::TableNextColumn();

			ImGui::BeginChild(actionsNum + i + 1, ImVec2(width, 25));

			if (ImGui::BeginTable("keys", 2))
			{
				ImGui::TableNextColumn();
				ImGui::Text(ax.name.c_str());
				ImGui::TableNextColumn();
				if (ax.name.find("MoveMouse") != std::string::npos) ImGui::Text(ax.fkey.ToString().c_str());
				else if (ax.GetListeningPreference() == Input::AxisListeningPreference::OneKey)
				{
					if (ImGui::Button(ax.fkey.ToString().c_str()))
					{
						Input::GetGlobalInput(0).SetAxisListening(ax.name, Input::AxisListeningMode::OnlyForward);
					}
				}
				else
				{
					if (ImGui::Button(ax.fkey.ToString().c_str()))
					{
						Input::GetGlobalInput(0).SetAxisListening(ax.name, Input::AxisListeningMode::ReplaceForward);
					}
					ImGui::SameLine();
					if (ImGui::Button(ax.bkey.ToString().c_str()))
					{
						Input::GetGlobalInput(0).SetAxisListening(ax.name, Input::AxisListeningMode::ReplaceBackward);
					}
				}
				ImGui::EndTable();
			}

			ImGui::EndChild();
			++i;
		}
		ImGui::EndTable();
	}
}

void UIKeyBinding::WindowBody()
{
	UIWindow::WindowBody();
	Input::BindSet bs = Input::GetGlobalInput(0).GetBindSet();
	
	DisplayActions(bs);
	DisplayAxes(bs);

	ImGui::Text("\n\n");

	if (Input::GetGlobalInput(0).IsListening())
	{
		ImGui::Text("Waiting for input...");
	}
	else
	{
		if (ImGui::Button("Save"))
		{
			bs.Save("bindsetTest.bs");
			Destroy();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			Input::GetGlobalInput(0).ApplyBindSet(backup);
			Destroy();
		}
	}
}
