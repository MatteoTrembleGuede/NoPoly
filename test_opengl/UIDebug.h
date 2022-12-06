#pragma once
#include "UIWindow.h"

class Shader;

class UIDebug : public UIWindow
{
private:

	static bool showTrespassing;
	static bool showSDF;
	static bool showComplexity;
	static bool showRenderChunks;
	static bool showBoundingVolumes;

	Shader* shader;

	void SendDebugConfig();

public:

	UIDebug(std::string _name, Shader* _shader);
	~UIDebug();

	virtual void WindowBody() override;
};

