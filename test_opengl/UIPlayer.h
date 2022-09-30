#pragma once
#include "UIWindow.h"
#include "RenderPlane.h"

class UIPlayer :
    public UIWindow
{
private:

	RenderPlane* plane;

public:

	UIPlayer(std::string _name, RenderPlane* _plane);
	~UIPlayer();

	void StartRecord(std::string _path, std::string _filename, void* _data);
	void StopRecord();
	virtual void WindowBody() override;
};

