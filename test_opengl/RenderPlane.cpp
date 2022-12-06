#include "RenderPlane.h"
#include <iostream>
#include "ViewportManager.h"
//#include <lock_guard>

#define RENDER_CHUNKS_NUM 10 //number of lines and columns

void RenderPlane::UpdateNumChunkToRender()
{
	int fps = 1.0f / mainFPSCounter.meanFrameTime;
	int targetFps = 200;

	if (fps > targetFps / 1.2f && targetChunkNumToRender > 0)
	{
		targetChunkNumToRender += targetChunkNumToRender / 16 + 1;
	}
	else if (fps > targetFps / 2)
	{
		targetChunkNumToRender++;
	}
	else if (fps < targetFps / 5)
	{
		targetChunkNumToRender /= 2;
	}
	else if (fps < targetFps / 4)
	{
		targetChunkNumToRender -= targetChunkNumToRender / 8 + 1;
	}
	else if (fps < targetFps / 2.5)
	{
		targetChunkNumToRender--;
	}

	targetChunkNumToRender = (targetChunkNumToRender < 0) ? 0 : (targetChunkNumToRender > RENDER_CHUNKS_NUM * RENDER_CHUNKS_NUM) ? RENDER_CHUNKS_NUM * RENDER_CHUNKS_NUM : targetChunkNumToRender;
}

RenderPlane::RenderPlane(Shader* _sceneShader) : shader("Shaders/VertexShader.shader", "Shaders/FragmentShader.shader"), sceneShader(_sceneShader)
{
	forceFullDraw = false;

	for (int i = 0; i < RENDER_CHUNKS_NUM * RENDER_CHUNKS_NUM; ++i)
	{
		Quad* quad = new Quad();
		quads.push_back(quad);
	}

	renderTexture = new RenderTexture(1, 1);
	renderTextureTmp = new RenderTexture(1, 1);
	SetPosAndSize(Vector3(0, 0, 0), Vector3(1, 1, 0), Vector3(1, 1, 0));
	mainFPSCounter.OnNewFrameRate.Bind(this, &RenderPlane::UpdateNumChunkToRender);

	Vector3 chunkSize = Vector3(1, 1, 0) / float(RENDER_CHUNKS_NUM);
	int i = 0;

	for (Quad*& quad : quads)
	{
		Vector3 offset = Vector3(i % RENDER_CHUNKS_NUM, i / RENDER_CHUNKS_NUM, 0.0f) / float(RENDER_CHUNKS_NUM);
		Vector3 uv = offset;
		Vertex* tlVert = quad->GetVertex(TopLeft);
		Vertex* trVert = quad->GetVertex(TopRight);
		Vertex* blVert = quad->GetVertex(BotLeft);
		Vertex* brVert = quad->GetVertex(BotRight);

		tlVert->position = (offset) * 2.0f - Vector3(1.0f, 1.0f, 0.0f);
		tlVert->u = uv.x;
		tlVert->v = uv.y;

		trVert->position = (offset + Vector3(chunkSize.x, 0, 0)) * 2.0f - Vector3(1.0f, 1.0f, 0.0f);
		trVert->u = uv.x + 1.0f / float(RENDER_CHUNKS_NUM);
		trVert->v = uv.y;

		blVert->position = (offset + Vector3(0, chunkSize.y, 0)) * 2.0f - Vector3(1.0f, 1.0f, 0.0f);
		blVert->u = uv.x;
		blVert->v = uv.y + 1.0f / float(RENDER_CHUNKS_NUM);

		brVert->position = (offset + chunkSize) * 2.0f - Vector3(1.0f, 1.0f, 0.0f);
		brVert->u = uv.x + 1.0f / float(RENDER_CHUNKS_NUM);
		brVert->v = uv.y + 1.0f / float(RENDER_CHUNKS_NUM);

		quad->UpdateGlSelf();
		++i;
	}
}

RenderPlane::~RenderPlane()
{

}

bool RenderPlane::Draw()
{
	int NumToRenderThisFrame;

	renderTextureTmp->BindAsTarget();

	if (forceFullDraw)
	{
		indexToDraw = 0;
		targetChunkNumToRender = RENDER_CHUNKS_NUM * RENDER_CHUNKS_NUM;
	}

	sceneShader->setInt("chunkNum", indexToDraw);
	sceneShader->setInt("chunkNumMax", RENDER_CHUNKS_NUM * RENDER_CHUNKS_NUM);
	NumToRenderThisFrame = (targetChunkNumToRender > RENDER_CHUNKS_NUM * RENDER_CHUNKS_NUM - indexToDraw) ?
		RENDER_CHUNKS_NUM * RENDER_CHUNKS_NUM - indexToDraw : targetChunkNumToRender;

	for (int i = 0; i < NumToRenderThisFrame; ++i)
	{
		quads[indexToDraw + i]->Draw();
	}
	renderTextureTmp->Unbind();

	shader.use();
	shader.setInt("_MainTex", 0);

	if (movieWriter && videoRecordDebug)
		videoRecordDebug->Bind(0);
	else
		renderTexture->Bind(0);

	finalQuad.Draw();

	mainFPSCounter.Update();

	indexToDraw = (indexToDraw + NumToRenderThisFrame);
	if (indexToDraw >= RENDER_CHUNKS_NUM * RENDER_CHUNKS_NUM)
	{
		indexToDraw = 0;

		RenderTexture* tmp = renderTexture;
		renderTexture = renderTextureTmp;
		renderTextureTmp = tmp;
		//renderTextureTmp->Clear();
		renderTextureTmp->Unbind();
		renderFPSCounter.Update();

		if (movieWriter)
		{
			unsigned int width, height;
			renderTexture->GetSize(width, height);
			unsigned char* pixels = new unsigned char[3 * width * height];
			renderTexture->BindAsTarget();
			glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			movieWriter->AddFrame(pixels);
			renderTexture->Unbind();
			delete[] pixels;
		}

		return true;
	}
	else
	{
		return false;
	}
}

void RenderPlane::SetPosAndSize(Vector3 _position, Vector3 _size, Vector3 _windowSize)
{
	_position.y = -_position.y;
	_size.y = -_size.y;

	renderTexture->SetSize(_windowSize.x, _windowSize.y);
	renderTextureTmp->SetSize(_windowSize.x, _windowSize.y);
	indexToDraw = 0;

	Vertex* tlVert = finalQuad.GetVertex(TopLeft);
	Vertex* trVert = finalQuad.GetVertex(TopRight);
	Vertex* blVert = finalQuad.GetVertex(BotLeft);
	Vertex* brVert = finalQuad.GetVertex(BotRight);

	tlVert->position = _position;
	trVert->position = _position + Vector3(_size.x, 0, 0);
	blVert->position = _position + Vector3(0, _size.y, 0);
	brVert->position = _position + _size;

	finalQuad.UpdateGlSelf();
}

void RenderPlane::StartMovie(std::string fileName, int frameRate)
{
	unsigned int width, height;
	renderTexture->GetSize(width, height);
	videoRecordDebug = Texture::LoadFromBuffer(ColorChannel::RGB, width, height);
	movieWriter = new Encoder(fileName, width, height, frameRate, videoRecordDebug);
	forceFullDraw = true;
}

void RenderPlane::StopMovie()
{
	delete movieWriter;
	movieWriter = nullptr;
	forceFullDraw = false;
	delete videoRecordDebug;
	videoRecordDebug = nullptr;
}
