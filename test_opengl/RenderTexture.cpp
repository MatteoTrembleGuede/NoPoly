#include "RenderTexture.h"
#include "ViewportManager.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

RenderTexture::RenderTexture(float texWidth, float texHeight) : 
	Texture(ColorChannel::RGBA, int(texWidth) - (int(texWidth) % 4), int(texHeight) - (int(texHeight) % 4))
{
	int w = int(texWidth);
	int h = int(texHeight);
	width = w - (w % 4);
	height = h - (h % 4);
	glGenFramebuffers(1, &glFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, glFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glID, 0);

	glGenRenderbuffers(1, &glRBO);
}

RenderTexture::~RenderTexture()
{
}

void RenderTexture::SetSize(float _width, float _height)
{
	int w = int(_width);
	int h = int(_height);
	/*width = w % 2 == 0 ? w : w - 1;
	height = h % 2 == 0 ? h : h - 1;*/
	width = w - (w % 4);
	height = h - (h % 4);
	UpdateBuffer();
}

void RenderTexture::BindAsTarget()
{
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, glFBO);
}

void RenderTexture::Unbind()
{
	float screenWidth, screenHeight;
	ViewportManager::GetScreenSize(screenWidth, screenHeight);
	glViewport(0, 0, screenWidth, screenHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTexture::Clear()
{
	BindAsTarget();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTexture::UpdateBuffer()
{
	BindAsTarget();
	Texture::UpdateBuffer();
	glBindRenderbuffer(GL_RENDERBUFFER, glRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glRBO);
	Unbind();
}
