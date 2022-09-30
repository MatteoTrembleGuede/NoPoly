#pragma once
#include "Delegate.h"

enum TimeMode
{
	Normal,
	Loop,
	MirroredLoop
};

static class Time
{
public:

	static Notify notifyLoopEnd;
	static bool playing;
	static TimeMode mode;

	static bool fixedFrameTime;
	static unsigned int frameRate;

	static double deltaTime;
	static double time;
	static double fullTime;
	static double speed;
	static double loopTime;

	static void Init();
	static void Reset();
	static void Update();
	static double GetTime();
	static double GetFrameTime() { return deltaTime; }
	static void StartFixedStepSequence();
	static void StopFixedStepSequence();
};

