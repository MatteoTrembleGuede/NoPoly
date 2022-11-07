#pragma once
#include "Input/Delegate.h"

class FPSCounter
{
private:

    float lastTime;
    float frameTime;
    float frameTimeSum;
    int frameCounter;
    float meanTime;
    float minFrameTime = 100.0f;

public:

    Notify OnNewFrameRate;
    const float& meanFrameTime;
    const float& minimumFrameTime;

    FPSCounter();

    void Update();
};
