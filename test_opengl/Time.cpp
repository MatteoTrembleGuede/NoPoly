#include "Time.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "ViewportManager.h"
#include "imgui/imgui.h"
#include "Input/Input.h"

bool Time::playing;
bool Time::fixedFrameTime;
TimeMode Time::mode;
double Time::deltaTime;
double Time::time;
double Time::fullTime;
double Time::speed;
double Time::loopTime;
unsigned int Time::frameRate;
Notify Time::notifyLoopEnd;

void Time::TogglePause()
{
	Time::playing = !Time::playing;
}

void Time::Init()
{
	playing = true;
	mode = TimeMode::Normal;
	fixedFrameTime = false;
	fullTime = 0;
	deltaTime = 0;
	loopTime = 10.0f;
	frameRate = 30.0f;
	Reset();

	Input::GetGlobalInput(0).AddAction("TogglePause", Input::Key(Input::KeyVal::SPACE));
	Input::GetGlobalInput(0).BindAction("TogglePause", Input::Mode::Press, &Time::TogglePause);
}

void Time::Reset()
{
	time = 0;
	speed = 1;
}

void Time::Update()
{
	double gltime = glfwGetTime();
	deltaTime = gltime - fullTime;
	fullTime = gltime;

	if (playing)
	{
		if (fixedFrameTime)
		{
			time += 1.0f / float(frameRate);
			bool condition = false;
			switch (mode)
			{
			case Normal:
			case Loop:
				condition = time >= loopTime;
				break;
			case MirroredLoop:
				condition = time >= (loopTime * 2.0f);
				break;
			}

			if (ViewportManager::IsSceneViewMaximized())
			{
				ImGui::OpenPopup("record progress");
				ImGui::BeginPopup("record progress");
				float t = Time::time;
				ImGui::SliderFloat("Time", &t, 0, Time::loopTime, "%f s");
				ImGui::EndPopup();
			}

			if (condition) StopFixedStepSequence();
		}
		else
		{
			time += deltaTime * speed;
		}
	}

	switch (mode)
	{
	case Loop:
		time = fmod(time, loopTime);
		break;
	case MirroredLoop:
		time = fmod(time, loopTime * 2.0);
		break;
	}

	if (mode != Normal && time < 0)
	{
		switch (mode)
		{
		case Loop:
			time += loopTime;
			break;
		case MirroredLoop:
			time += loopTime * 2.0;
			break;
		}
	}
}

double Time::GetTime()
{
	if (mode == MirroredLoop)
	{
		return time > loopTime ? loopTime - (time - loopTime) : time;
	}
	else
	{
		return time;
	}
}

void Time::StartFixedStepSequence()
{
	playing = true;
	time = 0;
	fixedFrameTime = true;
}

void Time::StopFixedStepSequence()
{
	fixedFrameTime = false;
	notifyLoopEnd();
	notifyLoopEnd.Clear();
}
