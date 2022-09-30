#include "FPSCounter.h"
#include "GLFW/glfw3.h"

void FPSCounter::Update()
{
	frameTime = glfwGetTime() - lastTime;
	lastTime += frameTime;
	frameTimeSum += frameTime;
	frameCounter++;

	if (frameCounter == 20)
	{
		frameCounter = 0;
		meanTime = frameTimeSum / 20.0f;
		frameTimeSum = 0;
		minFrameTime = meanTime < minFrameTime ? meanTime : minFrameTime;
		OnNewFrameRate();
	}
}

FPSCounter::FPSCounter() : meanFrameTime(meanTime), minimumFrameTime(minFrameTime)
{
	lastTime = glfwGetTime();
}
