#pragma once

#include <string>
#include "Texture.h"

class Writer;
struct Pixel;

class Encoder
{
public:

	Encoder(std::string fileName, int width, int height, int frameRate, Texture* videoRecordDebug);
	~Encoder();


	void AddFrame(unsigned char* pixels);

private:

#ifndef _DEBUG
	Writer* writer;
#endif

	Texture* displayTexture;
	int videoWidth, videoHeight;
	int inputWidth, inputHeight;

	float xRatio;
	float yRatio;

	bool forceInputResolution = true;

	Pixel Bilinear(unsigned char* pixels, int x, int y);
	Pixel Point(unsigned char* pixels, int x, int y);
	void SetRatioAndResolution();
};