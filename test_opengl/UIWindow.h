#pragma once
#include <string>
#include <list>
#include "imgui/imgui.h"
#include "Vector3.h"

struct ViewportSnapSection;
enum ViewportSnapSectionQuadrant;

class UIWindow
{
	friend class ViewportManager;
	friend class UIHost;

public:

	UIWindow();
	UIWindow(std::string _name);
	~UIWindow();

	virtual void Display();

	static void DisplayUI();
	static UIWindow* GetWindow(std::string name);

protected:

	static bool mustSkipDisplay;
	std::string name;
	unsigned int flags;
	void* host = nullptr;

	virtual void WindowBody();
	void Destroy();

private:

	ViewportSnapSection* snapSection;
	bool isDragged = false;
	bool isResized = false;
	bool isDestroyed = false;
	bool canResize = false;
	ViewportSnapSectionQuadrant sideDragged;

	static std::list<UIWindow*> windows;

	static bool MustSkip();
};



