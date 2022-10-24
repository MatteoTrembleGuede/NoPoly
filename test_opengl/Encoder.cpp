#include "Encoder.h"
#include <vector>
#include <iostream>

#ifndef _DEBUG
#include <opencv2/opencv.hpp>
#endif

//#include "Vector3.h"
//#include <opencv2/highgui.hpp>

struct Pixel
{
	unsigned char r;
	unsigned char g;
	unsigned char b;

	struct SwizzleXYZ
	{
		operator Pixel()
		{
			return Pixel{ *((unsigned char*)this-3), *((unsigned char*)this - 2), *((unsigned char*)this - 1) };
		}
	};
	
	struct SwizzleXZY
	{
		operator Pixel()
		{
			auto t = this - 1;
			return Pixel{ *((unsigned char*)t-3), *((unsigned char*)t - 1), *((unsigned char*)t - 2) };
		}
	};
	
	struct SwizzleYXZ
	{
		operator Pixel()
		{
			auto t = this - 2;
			return Pixel{ *((unsigned char*)t-2), *((unsigned char*)t - 3), *((unsigned char*)t - 1) };
		}
	};
	
	struct SwizzleYZX
	{
		operator Pixel()
		{
			auto t = this - 3;
			return Pixel{ *((unsigned char*)t-2), *((unsigned char*)t - 1), *((unsigned char*)t - 3) };
		}
	};
	
	struct SwizzleZXY
	{
		operator Pixel()
		{
			auto t = this - 4;
			return Pixel{ *((unsigned char*)t-1), *((unsigned char*)t - 3), *((unsigned char*)t - 2) };
		}
	};
	
	struct SwizzleZYX
	{
		operator Pixel()
		{
			auto t = this - 5;
			return Pixel{ *((unsigned char*)t-1), *((unsigned char*)t - 2), *((unsigned char*)t - 3) };
		}
	};

	SwizzleXYZ rgb;
	SwizzleXZY rbg;
	SwizzleYXZ grb;
	SwizzleYZX gbr;
	SwizzleZXY brg;
	SwizzleZYX bgr;

	Pixel Lerp(const Pixel& pix, float a)
	{
		a = std::min(1.0f, std::max(0.0f, a));
		Pixel res;
		res.r = roundf(pix.r * a + r * (1.0f - a));
		res.g = roundf(pix.g * a + g * (1.0f - a));
		res.b = roundf(pix.b * a + b * (1.0f - a));
		return res;
	}
};

#ifndef _DEBUG
class Writer
{
	cv::VideoWriter writer;
public:

	Writer(std::string fileName, int width, int height, int frameRate) :
		writer(fileName.c_str(), cv::VideoWriter::fourcc('a', 'v', 'c', '1'), frameRate, cv::Size(width, height))
		//writer(fileName.c_str(), cv::VideoWriter::fourcc('H', '2', '6', '4'), frameRate, cv::Size(width, height))
	{
		if (!writer.isOpened()) std::cout << "video writer couldn't create/open " << fileName << std::endl;
		else
		{
			//writer.set(cv::VIDEOWRITER_PROP_QUALITY, 50.0);
		}
	};
	~Writer() { writer.release(); };

	void Write(cv::Mat& matrix) { writer.write(matrix); };
};
#endif

void Encoder::SetRatioAndResolution()
{

#ifndef _DEBUG
	if (!writer)
	{
#endif
		if (inputHeight <= 240)
		{
			videoHeight = 240;
		}
		else if (inputHeight <= 360)
		{
			videoHeight = 360;
		}
		else if (inputHeight <= 480)
		{
			videoHeight = 480;
		}
		else if (inputHeight <= 720)
		{
			videoHeight = 720;
		}
		else if (inputHeight <= 1080)
		{
			videoHeight = 1080;
		}
		else if (inputHeight <= 2160)
		{
			videoHeight = 2160;
		}

		if (inputWidth <= videoHeight * 4 / 3)
		{
			videoWidth = videoHeight * 4 / 3;
		}
		else if (inputWidth <= videoHeight * 16 / 9)
		{
			videoWidth = videoHeight * 16 / 9;
		}
		else if (inputWidth <= videoHeight * 21 / 9)
		{
			videoWidth = videoHeight * 21 / 9;
		}
		else if (inputWidth <= videoHeight * 32 / 9)
		{
			videoWidth = videoHeight * 32 / 9;
		}

#ifndef _DEBUG
	}
#endif

	xRatio = inputWidth / float(videoWidth);
	yRatio = inputHeight / float(videoHeight);

	if (yRatio > xRatio)
	{
		xRatio = videoWidth / (inputWidth / yRatio);
		yRatio = 1.0f;
	}
	else
	{
		yRatio = videoHeight/ (inputHeight/ xRatio);
		xRatio = 1.0f;
	}
}

Encoder::Encoder(std::string fileName, int width, int height, int frameRate, Texture* videoRecordDebug) :
	displayTexture(videoRecordDebug),
	inputWidth(width),
	inputHeight(height)
{
	if (forceInputResolution)
	{
		videoWidth = inputWidth;
		videoHeight = inputHeight;
		xRatio = 1.0f;
		yRatio = 1.0f;
	}
	else
	{
		SetRatioAndResolution();
	}

#ifndef _DEBUG
	writer = new Writer(fileName, videoWidth, videoHeight, frameRate);
#endif
}

Encoder::~Encoder()
{
#ifndef _DEBUG
	delete writer;
#endif
}

Pixel Encoder::Bilinear(unsigned char* pixels, int x, int y)
{
	float u = ((x / float(videoWidth)) * 2.0f - 1.0f) * xRatio * 0.5f + 0.5f;
	float v = ((y / float(videoHeight)) * 2.0f - 1.0f) * yRatio * 0.5f + 0.5f;

	if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f) return Pixel{ 0, 0, 0 };

	float ax = u * inputWidth;
	float ay = v * inputHeight;
	int x1 = ax;
	int x2 = (x1 + 1) >= inputWidth ? x1 : (x1 + 1);
	ax = ax - x1;
	int y1 = ay;
	int y2 = (y1 + 1) >= inputHeight ? y1 : (y1 + 1);
	y1 = inputHeight - 1 - y1;
	y2 = inputHeight - 1 - y2;

	Pixel& pix00 = *(Pixel*)&pixels[(y1 * inputWidth + x1)*3];
	Pixel& pix01 = *(Pixel*)&pixels[(y2 * inputWidth + x1)*3];
	Pixel& pix10 = *(Pixel*)&pixels[(y1 * inputWidth + x2)*3];
	Pixel& pix11 = *(Pixel*)&pixels[(y2 * inputWidth + x2)*3];

	Pixel&& lpix1 = pix00.Lerp(pix01, ay);
	Pixel&& lpix2 = pix10.Lerp(pix11, ay);
	return lpix1.Lerp(lpix2, ax);
}

Pixel Encoder::Point(unsigned char* pixels, int x, int y)
{
	float u = ((x / float(videoWidth)) * 2.0f - 1.0f) * xRatio * 0.5f + 0.5f;
	float v = ((y / float(videoHeight)) * 2.0f - 1.0f) * yRatio * 0.5f + 0.5f;

	if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f) return Pixel{ 0, 0, 0 };

	int x1 = u * inputWidth;
	int y1 = v * inputHeight;
	y1 = inputHeight - 1 - y1;
	return *(Pixel*)&pixels[(y1 * inputWidth + x1) * 3];
}

void Encoder::AddFrame(unsigned char* pixels)
{
	if (displayTexture) displayTexture->UpdateBuffer(ColorChannel::RGB, inputWidth, inputHeight, pixels, true);

	unsigned char* newPix = new unsigned char[videoWidth * videoHeight * 3];

	for (int i = 0; i < videoHeight; ++i)
	{
		for (int j = 0; j < videoWidth; ++j)
		{
			//Pixel& npix = *(Pixel*)&newPix[(i * videoWidth + j) * 3];
			//npix = Bilinear(pixels, j, i);
			//npix = Point(pixels, j, i);

			*(Pixel*)&newPix[(i * videoWidth + j) * 3] = (*(Pixel*)&pixels[((videoHeight - 1 - i) * videoWidth + j) * 3]).bgr;
		}
	}

#ifndef _DEBUG
	cv::Mat frame(videoHeight, videoWidth, CV_8UC3, newPix);
	writer->Write(frame);
#endif
	delete[] newPix;
}
