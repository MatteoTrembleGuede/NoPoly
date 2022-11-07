#include "UIPlayer.h"
#include "Time.h"
#include "UIResourceBrowser.h"
#include "Input/Input.h"

UIPlayer::UIPlayer(std::string _name, RenderPlane* _plane) : UIWindow(_name), plane(_plane)
{
	Input::GetGlobalInput(0).AddAction("SaveRecord", Input::Key(Input::KeyVal::R));
	Input::GetGlobalInput(0).BindAction("SaveRecord", Input::Mode::Press, this, &UIPlayer::SaveRecord);
}

UIPlayer::~UIPlayer()
{
}

void UIPlayer::SaveRecord()
{
	if (Time::mode > TimeMode::Normal)
	{
		new UIResourceBrowser("Save video to", ".mp4,.mov,.avi", this, &UIPlayer::StartRecord, nullptr, true);
	}
}

void UIPlayer::StartRecord(std::string _path, std::string _filename, void* _data)
{
	Time::StartFixedStepSequence();
	Time::notifyLoopEnd.Bind(this, &UIPlayer::StopRecord);
	plane->StartMovie(_path + _filename, Time::frameRate);
}

void UIPlayer::StopRecord()
{
	plane->StopMovie();
}

void UIPlayer::WindowBody()
{
	UIWindow::WindowBody();

	if (ImGui::BeginTable("time", 3))
	{
		ImGui::TableNextColumn();
		if (!Time::fixedFrameTime && ImGui::Button(Time::playing ? "Pause" : "Play"))
		{
			Time::TogglePause();
		}

		ImGui::SameLine();
		if (!Time::playing)
		{
			float t = Time::time;
			ImGui::DragFloat("Time", &t, 0.01, -FLT_MAX, FLT_MAX, "%f");
			Time::time = t;
		}
		else
		{
			float t = Time::GetTime();
			switch (Time::mode)
			{
			case Normal:
				ImGui::Text((std::to_string(Time::GetTime()) + " s").c_str());
				break;
			case Loop:
			case MirroredLoop:
				ImGui::SliderFloat("Time", &t, 0, Time::loopTime, "%f s");
				break;
			}
		}

		if (!Time::fixedFrameTime)
		{
			ImGui::TableNextColumn();
			float s = Time::speed;
			ImGui::DragFloat("Speed", &s, 0.01);
			Time::speed = s;

			ImGui::TableNextColumn();
			if (ImGui::Button("Reset"))
			{
				Time::Reset();
			}

			ImGui::TableNextColumn();
			int selectedMode = (int)Time::mode;
			const char* modes[3]{ "Normal" , "Loop" , "Mirrored Loop" };

			ImGui::Combo("Time mode", &selectedMode, modes, 3);

			if (selectedMode != (int)Time::mode)
			{
				Time::mode = (TimeMode)selectedMode;
			}

			ImGui::TableNextColumn();
			if (Time::mode != Normal)
			{
				float t = Time::loopTime;
				ImGui::DragFloat("Loop time", &t, 0.01, 0.01, FLT_MAX, "%f");
				Time::loopTime = t;

				ImGui::TableNextColumn();

				ImGui::TableNextColumn();
				int f = Time::frameRate;
				ImGui::DragInt("Frame rate", &f, 1, 0, 60);
				Time::frameRate = f;

				ImGui::TableNextColumn();
				if (ImGui::Button("Record Video"))
				{
					SaveRecord();
					//new UIResourceBrowser("Save video to", ".mp4", this, &UIPlayer::StartRecord, nullptr, true);
				}
			}
		}
		else
		{
			ImGui::TableNextColumn();
			ImGui::Text("Recording");
		}
		ImGui::EndTable();
	}
}
