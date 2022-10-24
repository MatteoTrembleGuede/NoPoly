#pragma once

#include <glm/vec3.hpp>
#include <string>

enum ColorChannel
{
	R = 0x2002,
	RG = 0x8227,
	RGB = 0x1907,
	RGBA = 0x1908
};

class Texture
{
protected:

	unsigned char* buffer;
	unsigned int height;
	unsigned int width;
	ColorChannel channelFormat;
	unsigned int glID;
	std::string path = "";

	Texture();
	Texture(ColorChannel format, unsigned int imageWidth, unsigned int imageHeight, unsigned char* colorBuffer = nullptr);

public:

	~Texture();

	bool Bind(unsigned int layout = 0);
	unsigned char* GetRawBuffer() { return buffer; }
	unsigned int GetGlID() { return glID; }
	void GetSize(unsigned int& outWidth, unsigned int& outHeight) { outWidth = width; outHeight = height; }
	bool GetColor(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b);
	bool SetColor(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	virtual void UpdateBuffer();
	void UpdateBuffer(ColorChannel _format, int _width, int _height, unsigned char* _colorBuffer, bool copy);
	std::string GetPath() { return path; }
	
	static Texture* LoadFromFile(const char* path);
	static Texture* LoadFromBuffer(ColorChannel format, unsigned int imageWidth, unsigned int imageHeight, unsigned char* colorBuffer = nullptr);
};

