#pragma once

#include "Quad.h"
#include <vector>
#include <thread>
#include <mutex>
#include "Vector3.h"
#include "RenderTexture.h"
#include "Shader.h"
#include "FPSCounter.h"
#include "Encoder.h"
#include "time.h"

class RenderPlane
{

private:

	int indexToDraw = 0;
	Quad finalQuad;
	Shader shader;
	Shader* sceneShader;
	std::vector<Quad*> quads;
	RenderTexture* renderTexture;
	RenderTexture* renderTextureTmp;
	Texture* videoRecordDebug;
	FPSCounter mainFPSCounter;
	FPSCounter renderFPSCounter;
	int targetChunkNumToRender = 0;

	void UpdateNumChunkToRender();

	Encoder* movieWriter = nullptr;
	bool forceFullDraw;

public:

	RenderPlane(Shader* _sceneShader);
	~RenderPlane();
	bool Draw();
	void SetPosAndSize(Vector3 position, Vector3 size, Vector3 _windowSize);

	void StartMovie(std::string fileName, int frameRate);
	void StopMovie();
};

