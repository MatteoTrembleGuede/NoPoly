#pragma once

#include "UIWindow.h"
#include "Shader.h"
#include "RenderPlane.h"
#include <list>

struct Edge;
struct GLFWwindow;
enum EdgeOrientation;

typedef enum ViewportSnapSectionQuadrant
{
	outside,
	left,
	right,
	top,
	bot,
	center
}ViewportSnapSectionQuadrant;

typedef struct ViewportSnapSection
{
	friend struct Edge;
	friend class ViewportManager;

public:


	void GetSizeAndPosition(ImVec2& outSize, ImVec2& outPos) const;
	ViewportSnapSectionQuadrant Contains(const ImVec2& position, bool noCenter = false) const;
	ViewportSnapSectionQuadrant GetDirectionFromSectionCenter(const ImVec2& _position) const;
	void PushSide(ViewportSnapSectionQuadrant side, float value);

private:
	Edge* top;
	Edge* bot;
	Edge* left;
	Edge* right;

	ViewportSnapSection* parent = nullptr;

	ViewportSnapSection();
	~ViewportSnapSection();

	void SetSizeAndPosition(ImVec2 outSize, ImVec2 outPos);
	void Destroy();
}ViewportSnapSection;

static class ViewportManager
{
public:

	static float screenResolutionScale;
	static UIWindow* draggedWindow;
	static ImVec2 lSize;
	static ImVec2 lPos;

	static void Init(Shader* _shader, RenderPlane* _quad);
	static GLFWwindow* CreateWindow();
	static void RebindEdges(Edge* oldEdge, Edge* replacementEdge);
	static void GetScreenSize(float& outWidth, float& outHeight);
	static void RemoveEdge(Edge* edge);
	static void RemoveSection(ViewportSnapSection* section);
	static void Snap(UIWindow* window, ImVec2 position);
	static void Snap(UIWindow* window, UIWindow* parentWindow);
	static void DebugDrawEdges();
	static void AdjustQuad();
	static float FixNextPosition(Edge* edge, float position);
	static void MaximizeSceneView(bool _maximize);
	static bool IsSceneViewMaximized();
	static bool IsQuadContaining(ImVec2 pos);
	static void GetSceneViewPosAndSize(ImVec2& outSize, ImVec2& outPos);
	static UIWindow* GetUIWindowInSection(ViewportSnapSection* _section);
	static GLFWwindow* GetWindow() { return window; };
	static void SaveLayout();
	static bool LoadLayout();
	static void DragWindow(UIWindow* _window);
	static void StopDragging();
	static void Update();

private:

	static Shader* shader;
	static RenderPlane* sceneQuad;
	static std::list<Edge*> edges;
	static std::list<ViewportSnapSection*> sections;
	static float screenWidth, screenHeight;
	static GLFWwindow* window;
	static bool maximizeSceneView;

	static ViewportSnapSection* GetSectionAndQuadrant(ImVec2 position, ViewportSnapSectionQuadrant& outQuadrant, bool noCenter = false);
	static ViewportSnapSection* AddSection(ImVec2 position);

	static void WindowSizeCallback(GLFWwindow* window, int width, int height);
};

