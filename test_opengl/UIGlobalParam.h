#pragma once
#include "UIWindow.h"
#include "Shader.h"



class UIGlobalParam :
    public UIWindow
{

private:

	Shader* shader;

	static bool applyPRMNC;
	static float AORadius;
	static float AOStrength;
	static float normalRadius;
	static float shadowSharpness;

	void SendGlobalConfig();

public:

    UIGlobalParam(std::string _name, Shader* _shader);
    ~UIGlobalParam();

	virtual void WindowBody() override;

};

