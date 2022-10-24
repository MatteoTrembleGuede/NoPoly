#include "Texture.h"
#include "stb_image/stb_image.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>

Texture::Texture()
{
	buffer = nullptr;
	glID = 0;
	channelFormat = ColorChannel::RGBA;
	height = 0;
	width = 0;
	glGenTextures(1, &glID);
}

Texture::Texture(ColorChannel format, unsigned int imageWidth, unsigned int imageHeight, unsigned char* colorBuffer /*= nullptr*/)
{
	buffer = colorBuffer;
	channelFormat = format;
	width = imageWidth;
	height = imageHeight;
	glGenTextures(1, &glID);
	UpdateBuffer();
}

Texture::~Texture()
{
	glDeleteTextures(1, &glID);
	delete buffer;
}

bool Texture::GetColor(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b)
{
	unsigned int step = channelFormat == RGBA ? 4 : (channelFormat == RGB ? 3 : (channelFormat == RG ? 2 : 1));

	if (x >= 0 && x < width
		&& y >= 0 && y < height)
	{
		unsigned int pixelID = (y * width + x) * step;
		switch (channelFormat)
		{
		case RGBA:
		case RGB:
			b = buffer[pixelID + 2];
		case RG:
			g = buffer[pixelID + 1];
		case R:
			r = buffer[pixelID];
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Texture::SetColor(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	unsigned int step = channelFormat == RGBA ? 4 : (channelFormat == RGB ? 3 : (channelFormat == RG ? 2 : 1));

	if (x >= 0 && x < width
		&& y >= 0 && y < height)
	{
		unsigned int pixelID = (y * width + x) * step;

		switch (channelFormat)
		{
		case RGBA:
		case RGB:
			buffer[pixelID + 2] = b;
		case RG:
			buffer[pixelID + 1] = g;
		case R:
			buffer[pixelID] = r;
		}
		return true;
	}
	else
	{
		return false;
	}
}

void Texture::UpdateBuffer()
{
	glBindTexture(GL_TEXTURE_2D, glID);
	glTexImage2D(GL_TEXTURE_2D, 0, channelFormat, width, height, 0, channelFormat, GL_UNSIGNED_BYTE, buffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Texture::UpdateBuffer(ColorChannel _format, int _width, int _height, unsigned char* _colorBuffer, bool copy)
{
	delete[] buffer;
	buffer = nullptr;
	width = _width;
	height = _height;
	channelFormat = _format;
	unsigned int step = channelFormat == RGBA ? 4 : (channelFormat == RGB ? 3 : (channelFormat == RG ? 2 : 1));
	if (copy)
	{
		//std::cout << "w = " << width << " h = " << height << " s = " << step << " w*h*s = " << width * height * step << std::endl;
		buffer = new unsigned char[width * height * step];
		memcpy(buffer, _colorBuffer, width * height * step);
	}
	else
		buffer = _colorBuffer;
	UpdateBuffer();
}

Texture* Texture::LoadFromFile(const char* path)
{
	int channelNumber;
	ColorChannel channelFormat;
	int width, height;

	unsigned char* buffer = stbi_load(path, &width, &height, &channelNumber, 3); // forcing 3 channels because ImGui renders black images if less
	channelNumber = 3;

	if (!buffer) return nullptr;

	if (channelNumber == 1)
	{
		channelFormat = ColorChannel::R;
	}
	else if (channelNumber == 2)
	{
		channelFormat = ColorChannel::RG;
	}
	else if (channelNumber == 3)
	{
		channelFormat = ColorChannel::RGB;
	}
	else
	{
		channelFormat = ColorChannel::RGBA;
	}

	Texture* tex = new Texture(channelFormat, width, height, buffer);
	tex->path = path;

	return tex;
}

Texture* Texture::LoadFromBuffer(ColorChannel format, unsigned int imageWidth, unsigned int imageHeight, unsigned char* colorBuffer /*= nullptr*/)
{
	return new Texture(format, imageWidth, imageHeight, colorBuffer);
}

bool Texture::Bind(unsigned int layout /*= 0*/)
{
	glActiveTexture(GL_TEXTURE0 + layout);
	glBindTexture(GL_TEXTURE_2D, glID);
	return true;
}
